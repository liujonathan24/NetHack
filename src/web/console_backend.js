/* console_backend.js — run the NetHack web console entirely client-side.
 *
 * The local app (tools/play_server.py) serves the same UI (map.html + console.js)
 * from a Flask server. This file replaces that server with a fetch() shim backed
 * by the WASM engine, so the identical page runs on a static host (GitHub Pages)
 * with no runtime. It implements the same JSON endpoints console.js already calls
 * (/catalog /current /reset /step /live /undo /mark /restore_mark /modify
 * /record_*), so console.js is used UNCHANGED.
 *
 * Undo / Checkpoint / Restore are done by deterministic replay of a (seed, tune,
 * curriculum, action-journal) — the engine is seed-deterministic, so replay
 * reproduces the exact state without needing a serialized snapshot.
 */
(function () {
  "use strict";

  const OPTS =
    "autopickup,color,disclose:+i +a +v +g +c +o,mention_walls,nobones," +
    "nocmdassist,nolegacy,nosparkle,pickup_burden:unencumbered," +
    "pickup_types:$?!/,runmode:teleport,showexp,showscore,time,name:Agent-Val-hum-neu-fem";

  const ROWS = 21, COLS = 79;
  const TTY_ROWS = 24, TTY_COLS = 80;   // the real terminal, where menus/prompts are drawn

  /* Knob catalog — ported verbatim from tools/play_server.py (_META/_GROUPS) so
     the controls are exactly the ones the local app shows. */
  const GROUPS = ["Vision", "Stat-based", "Dungeon & spawns"];
  const DEFAULT_META = { group: "Stat-based", kind: "scale", reset: false, lo: 0, hi: 3, step: 0.25, default: 1, note: "" };
  const META = {
    vision_radius:            { group: "Vision", kind: "int",  reset: false, lo: 0, hi: 15, step: 1, default: 0, note: "0 = vanilla; only matters in the dark" },
    reveal_map:               { group: "Vision", kind: "bool", reset: false, default: 0, note: "on = reveal whole map incl. walls + live monsters" },
    dmg_to_player_scale:      { group: "Stat-based", kind: "scale", reset: false, lo: 0, hi: 4, step: 0.25, default: 1 },
    dmg_by_player_scale:      { group: "Stat-based", kind: "scale", reset: false, lo: 0, hi: 4, step: 0.25, default: 1 },
    player_hp_scale:          { group: "Stat-based", kind: "scale", reset: false, lo: 0.25, hi: 4, step: 0.25, default: 1, note: "HP gained on level-up" },
    hp_regen_scale:           { group: "Stat-based", kind: "scale", reset: false, lo: 0, hi: 8, step: 0.5, default: 1 },
    hunger_rate_scale:        { group: "Stat-based", kind: "scale", reset: false, lo: 0, hi: 5, step: 0.25, default: 1 },
    xp_gain_scale:            { group: "Stat-based", kind: "scale", reset: false, lo: 0, hi: 10, step: 0.5, default: 1 },
    room_density:             { group: "Dungeon & spawns", kind: "scale", reset: true,  lo: 0.0, hi: 1.5, step: 0.05, default: 1, note: "RESET to regenerate the floor" },
    monster_difficulty_scale: { group: "Dungeon & spawns", kind: "scale", reset: true,  lo: 0, hi: 10, step: 0.5, default: 1, note: "RESET to reshape this floor; live for new spawns" },
    ongoing_spawn_scale:      { group: "Dungeon & spawns", kind: "scale", reset: false, lo: 0, hi: 20, step: 0.5, default: 1 },
    monster_speed_scale:      { group: "Dungeon & spawns", kind: "scale", reset: false, lo: 0, hi: 4, step: 0.25, default: 1 },
    mob_spawn:                { group: "Dungeon & spawns", kind: "scale", reset: true,  lo: 0, hi: 3, step: 0.25, default: 1, note: "initial sleeping monsters per room; 0 = none" },
    trap_density:             { group: "Dungeon & spawns", kind: "scale", reset: true,  lo: 0, hi: 3, step: 0.25, default: 1, note: "traps per room; 0 = none" },
    locked_door:              { group: "Dungeon & spawns", kind: "scale", reset: true,  lo: 0, hi: 3, step: 0.25, default: 1, note: "door-lock chance; 0 = never locked" },
    corridor_connectivity:    { group: "Dungeon & spawns", kind: "scale", reset: true,  lo: 0, hi: 3, step: 0.25, default: 1, note: "extra/redundant corridors between rooms" },
    room_size:                { group: "Dungeon & spawns", kind: "scale", reset: true,  lo: 0.25, hi: 3, step: 0.25, default: 1, note: "room dimensions" },
  };
  /* whitelisted modify fields (mirror play_server _MODIFY_BOUNDS keys). */
  const MODIFY_FIELDS = ["hp", "max_hp", "gold", "xp_level", "hunger"];

  /* ---- engine handle + state ---- */
  let M = null, F = {}, obs = 0;
  let names = [], nameIdx = {};
  let curTune = {};                    // live knob values (name -> value)
  let seed = 42;
  let curriculum = { on: false, floor: 1 };   // "six-level curriculum" toggle
  /* Curriculum geometry resolved from the live (seed-dependent) dungeon table at
     each start — mirrors CurriculumEngineEnv.reset in nethack_core/. */
  const CURR_SHALLOW_HI = 3, CURR_DEEP = [48, 49, 50];
  let currGeo = null;   // {dod, geh, gehStart, shallowHi, deepLo, deepHi}
  // Replay journal since the last reset + the reset context to rebuild from.
  let journal = [];                    // {t:'step',keys} | {t:'live',name,val} | {t:'modify',changes}
  let markLen = null;                  // checkpoint = a journal length to replay to
  let resetCtx = { seed: 42, tune: {}, curriculum: { on: false, floor: 1 } };

  const ready = NetHackModule().then((mod) => { M = mod; _wrap(); });

  function _wrap() {
    const cw = (n, r, a, o) => M.cwrap(n, r, a, o);
    F.newObs   = cw("nleweb_new_obs", "number", []);
    F.start    = cw("nleweb_start", "number", ["number", "number", "string"], { async: true });
    F.step     = cw("nleweb_step", "number", ["number", "number"], { async: true });
    F.chars    = cw("nleweb_chars", "number", ["number"]);
    F.colors   = cw("nleweb_colors", "number", ["number"]);
    F.blstats  = cw("nleweb_blstats", "number", ["number"]);
    F.message  = cw("nleweb_message", "number", ["number"]);
    F.misc     = cw("nleweb_misc", "number", ["number"]);
    F.ttyChars = cw("nleweb_tty_chars", "number", ["number"]);
    F.invStrs  = cw("nleweb_inv_strs", "number", ["number"]);
    F.invLets  = cw("nleweb_inv_letters", "number", ["number"]);
    F.invCls   = cw("nleweb_inv_oclasses", "number", ["number"]);
    F.invSize  = cw("nleweb_inv_size", "number", []);
    F.invLen   = cw("nleweb_inv_str_len", "number", []);
    F.done     = cw("nleweb_done", "number", ["number"]);
    F.tuneCount= cw("nleweb_tune_count", "number", []);
    F.tuneName = cw("nleweb_tune_name", "number", ["number"]);
    F.setTune  = cw("nleweb_set_tune", null, ["number", "number"]);
    F.clearTune= cw("nleweb_clear_tune", null, []);
    F.getTune  = cw("nleweb_get_tune", "number", ["number"]);
    F.liveTune = cw("nleweb_live_tune", null, ["number", "number"]);
    F.setState = cw("nleweb_set_state", "number", ["string", "number"]);
    F.gotoDepth= cw("nleweb_goto_depth", "number", ["number"]);
    F.gotoAbs  = cw("nleweb_goto_abs", "number", ["number", "number"], { async: true });
    F.seat     = cw("nleweb_seat_on_stair", "number", ["number"]);
    F.onStair  = cw("nleweb_hero_on_stair", "number", []);
    F.levelUp  = cw("nleweb_level_up", "number", ["number"]);
    F.numDgn   = cw("nleweb_num_dungeons", "number", []);
    F.dgnInfo  = cw("nleweb_dungeon_info", "number", ["number", "number", "number", "number", "number"]);
    const n = F.tuneCount();
    for (let i = 0; i < n; i++) { const nm = M.UTF8ToString(F.tuneName(i)); names.push(nm); nameIdx[nm] = i; }
    names.forEach((nm) => { curTune[nm] = (META[nm] || DEFAULT_META).default; });
    obs = F.newObs();
  }

  /* ---- low-level engine ops (no journal bookkeeping) ---- */
  /* misc = [in_yn_function, in_getlin, waiting_for_space]. */
  function _misc() { const p = F.misc(obs) >> 2; return [M.HEAP32[p], M.HEAP32[p + 1], M.HEAP32[p + 2]]; }
  /* One row of the 24x80 terminal as text. */
  function _ttyRow(y) {
    const p = F.ttyChars(obs) + y * TTY_COLS;
    let s = "";
    for (let x = 0; x < TTY_COLS; x++) { const c = M.HEAPU8[p + x]; s += (c >= 32 && c < 127) ? String.fromCharCode(c) : " "; }
    return s;
  }
  /* True when the engine is showing something the user must answer or read:
     a yn/getlin prompt, or a menu. NOT a bare --More--, which we flush. */
  function _prompting() {
    const m = _misc();
    return !!(m[0] || m[1] || (m[2] && !_atMore()));
  }
  /* A genuine --More-- always writes that literal marker onto tty row 0.
     Menus (inventory, pickup/drop selection, the # command list) ALSO set
     waiting_for_space, so the flag alone cannot tell them apart — ESCing on the
     flag tore every menu down before the user could answer it. */
  function _atMore() { return _ttyRow(0).indexOf("--More--") >= 0; }
  async function _settle() {
    for (let i = 0; i < 16; i++) {
      const m = _misc();
      if (m[2] && !m[0] && !m[1] && _atMore()) await F.step(obs, 27); // ESC flushes --More--
      else break;
    }
  }
  async function _rawStepKeys(keys) {
    for (const ch of keys) {
      if (await _currBoundary(ch)) continue;   // curriculum stair redirect
      await F.step(obs, ch.charCodeAt(0));
    }
    await _settle();
  }
  function _rawLive(name, val) { if (name in nameIdx) F.liveTune(nameIdx[name], val); }
  function _currResolve() {
    const t = _dungeonTable();
    const dod = t.find((d) => /Dungeons of Doom/.test(d.name));
    const geh = t.find((d) => /Gehennom/.test(d.name));
    const gehMax = geh.depth_start + geh.num - 1;
    const clamp = (d) => Math.max(geh.depth_start, Math.min(gehMax, d));
    currGeo = {
      dod: dod.dnum, geh: geh.dnum, gehStart: geh.depth_start,
      shallowHi: CURR_SHALLOW_HI,
      deepLo: clamp(CURR_DEEP[0]), deepHi: clamp(CURR_DEEP[CURR_DEEP.length - 1]),
    };
    return currGeo;
  }
  function _heroAbs() {
    const b = F.blstats(obs) >> 2;
    return { dnum: M.HEAP32[b + 23], depth: M.HEAP32[b + 12] };
  }
  async function _currJump(dnum, dlevel) {
    await F.gotoAbs(dnum, dlevel);
    await F.step(obs, 27);
    await _settle();
  }
  async function _rawGotoFloor(floor) {
    const g = currGeo || _currResolve();
    if (floor <= 3) await _currJump(g.dod, floor);
    else await _currJump(g.geh, (g.deepLo + (floor - 4)) - g.gehStart + 1);
  }
  async function _currBoundary(ch) {
    if (!resetCtx.curriculum.on || (ch !== ">" && ch !== "<")) return false;
    /* Inside a menu or prompt, '>' means "next page" and '<' "previous" — it is
       not a stair command, so redirecting there would teleport the hero out of
       an open menu. */
    if (_prompting()) return false;
    const g = currGeo || _currResolve();
    const { dnum, depth } = _heroAbs();
    const st = F.onStair();                       // +1 down stair, -1 up stair, 0 none
    if (ch === ">" && dnum === g.dod && depth === g.shallowHi && st === 1) {
      await _currJump(g.geh, g.deepLo - g.gehStart + 1);
      return true;
    }
    if (ch === "<" && dnum === g.geh && depth === g.deepLo && st === -1) {
      await _currJump(g.dod, g.shallowHi);
      return true;
    }
    return false;
  }
  async function _rawModify(changes) {
    const c = Object.assign({}, changes);
    const depth = c.goto_depth; delete c.goto_depth;
    const levels = c.level_up; delete c.level_up;
    for (const k of Object.keys(c)) if (MODIFY_FIELDS.indexOf(k) >= 0) F.setState(k, c[k] | 0);
    if (levels != null) F.levelUp(levels | 0);
    if (depth != null) { F.gotoDepth(depth | 0); await F.step(obs, 27); F.seat(1); await F.step(obs, 27); }
    await F.step(obs, 18);   // ctrl-R: apply deferred pokes + redraw, no game turn
    await _settle();
  }
  async function _doStart(useSeed, useTune, useCurr) {
    F.clearTune();
    names.forEach((nm) => { const v = (nm in useTune) ? useTune[nm] : (META[nm] || DEFAULT_META).default; F.setTune(nameIdx[nm], v); });
    currGeo = null;                                 // dungeon table is per-game
    await F.start(obs, useSeed >>> 0, OPTS);
    await F.step(obs, 46); await F.step(obs, 46);   // two '.' waits, like play_server.reset
    await _settle();
    _currResolve();
    if (useCurr && useCurr.on) await _rawGotoFloor(useCurr.floor);
  }
  async function _replayTo(len) {
    const saved = journal.slice(0, len);
    journal = [];
    await _doStart(resetCtx.seed, resetCtx.tune, resetCtx.curriculum);
    for (const e of saved) {
      if (e.t === "step") { await _rawStepKeys(e.keys); }
      else if (e.t === "live") { _rawLive(e.name, e.val); await F.step(obs, 18); await _settle(); }
      else if (e.t === "modify") { await _rawModify(e.changes); }
      journal.push(e);
    }
  }

  /* ---- frame (the _payload shape play_server returns) ---- */
  function _dungeonTable() {
    const n = F.numDgn(), t = [];
    const nm = M._malloc(24), ds = M._malloc(4), nu = M._malloc(4);
    for (let i = 0; i < n; i++) { F.dgnInfo(i, nm, 24, ds, nu); t.push({ dnum: i, name: M.UTF8ToString(nm), depth_start: M.HEAP32[ds >> 2], num: M.HEAP32[nu >> 2] }); }
    M._free(nm); M._free(ds); M._free(nu); return t;
  }
  function _frame() {
    const cp = F.chars(obs), kp = F.colors(obs);
    const map = [], colors = [];
    for (let y = 0; y < ROWS; y++) {
      let s = ""; const crow = [];
      for (let x = 0; x < COLS; x++) { const ch = M.HEAPU8[cp + y * COLS + x]; s += (ch >= 32 && ch < 127) ? String.fromCharCode(ch) : " "; crow.push(M.HEAPU8[kp + y * COLS + x]); }
      map.push(s); colors.push(crow);
    }
    const b = F.blstats(obs) >> 2;
    const status = { hp: M.HEAP32[b + 10], max_hp: M.HEAP32[b + 11], ac: M.HEAP32[b + 16], dlvl: M.HEAP32[b + 12], gold: M.HEAP32[b + 13], xp_lvl: M.HEAP32[b + 18] };
    const mp = F.message(obs); let msg = ""; for (let i = 0; i < 256; i++) { const c = M.HEAPU8[mp + i]; if (!c) break; msg += String.fromCharCode(c); }
    const tune = {}; names.forEach((nm) => { tune[nm] = F.getTune(nameIdx[nm]); });
    const undos = journal.reduce((a, e) => a + (e.t === "step" ? 1 : 0), 0);
    return { map, colors, message: msg, status, tune, done: !!F.done(obs), recording: null, undos,
             marked: markLen != null, popup: _popup(), inventory: _inventory() };
  }

  /* The popup: whatever the engine is asking the user, scraped off the terminal.
     NetHack draws menus and prompts as an overlay on the 24x80 tty, so the rows
     that differ from blank ARE the popup. Returned only while something is
     actually pending, so the page can hide the box the rest of the time. */
  let quietTty = null;   // the terminal as it looked with nothing pending
  function _ttyAll() { const r = []; for (let y = 0; y < TTY_ROWS; y++) r.push(_ttyRow(y)); return r; }
  function _popup() {
    const now = _ttyAll();
    if (!_prompting()) { quietTty = now; return null; }
    /* A menu is drawn as an overlay ON TOP of whatever the terminal already
       held, so scraping non-blank rows also drags in the background underneath
       (e.g. the startup splash to the left of a right-hand menu). Diff against
       the last settled frame instead: the cells that changed ARE the popup. */
    const prev = quietTty;
    const diff = (y, x) => now[y][x] !== ((prev && prev[y] && prev[y][x]) || " ");
    /* Two passes. Columns come from rows 1..23 only, because tty row 0 is
       NetHack's message line: it changes on every command and would stretch the
       box to full width, dragging the background back in. Rows are then taken
       over the WHOLE screen but restricted to those columns, so a menu that
       overlays row 0 (its first group header often does) is still included. */
    let c0 = TTY_COLS, c1 = -1;
    for (let y = 1; y < TTY_ROWS; y++)
      for (let x = 0; x < TTY_COLS; x++)
        if (diff(y, x)) { if (x < c0) c0 = x; if (x > c1) c1 = x; }
    if (c1 < 0) {   // nothing changed below the message line — no popup to show
      return null;
    }
    let r0 = TTY_ROWS, r1 = -1;
    for (let y = 0; y < TTY_ROWS; y++)
      for (let x = c0; x <= c1; x++)
        if (diff(y, x)) { if (y < r0) r0 = y; if (y > r1) r1 = y; break; }
    if (r1 < 0) return null;
    const box = [];
    for (let y = r0; y <= r1; y++) box.push(now[y].slice(c0, c1 + 1).replace(/\s+$/, ""));
    while (box.length && !box[0]) box.shift();
    while (box.length && !box[box.length - 1]) box.pop();
    return box.length ? box : null;
  }

  /* Carried inventory, grouped by object class. The engine refreshes inv_* every
     turn on its own (the rl port walks `invent` rather than popping a menu), so
     this is a passive read — no keystroke, no menu, nothing to dismiss. */
  const OCLASS_NAMES = {
    2: "Weapons", 3: "Armor", 4: "Rings", 5: "Amulets", 6: "Tools", 7: "Food",
    8: "Potions", 9: "Scrolls", 10: "Spellbooks", 11: "Wands", 12: "Coins",
    13: "Gems", 14: "Rocks", 15: "Balls", 16: "Chains", 17: "Venom",
  };
  function _inventory() {
    const n = F.invSize(), len = F.invLen();
    const sp = F.invStrs(obs), lp = F.invLets(obs), cp = F.invCls(obs);
    const groups = [], byCls = {};
    for (let i = 0; i < n; i++) {
      const letter = M.HEAPU8[lp + i];
      if (!letter) break;                       // slots are packed; 0 terminates
      let text = "";
      for (let j = 0; j < len; j++) { const c = M.HEAPU8[sp + i * len + j]; if (!c) break; text += String.fromCharCode(c); }
      const cls = M.HEAPU8[cp + i];
      const name = OCLASS_NAMES[cls] || "Other";
      if (!byCls[name]) { byCls[name] = { name: name, cls: cls, items: [] }; groups.push(byCls[name]); }
      byCls[name].items.push({ letter: String.fromCharCode(letter), text: text });
    }
    groups.sort((a, b) => a.cls - b.cls);       // NetHack's own class order
    return groups;
  }

  /* ---- endpoint handlers (return {status, body}) ---- */
  function _catalog() {
    const out = names.map((nm) => Object.assign({}, DEFAULT_META, META[nm] || {}, { name: nm, note: (META[nm] || {}).note || "" }));
    const a = out.findIndex((m) => m.name === "monster_difficulty_scale");
    const bIdx = out.findIndex((m) => m.name === "monster_speed_scale");
    if (a >= 0 && bIdx >= 0) { const t = out[a]; out[a] = out[bIdx]; out[bIdx] = t; }
    return { status: 200, body: { groups: GROUPS, knobs: out } };
  }
  async function _reset(body) {
    const s = (body && Number.isFinite(+body.seed)) ? Math.trunc(+body.seed) : seed;
    seed = s;
    const tune = {}; if (body && body.tune) for (const k in body.tune) tune[k] = +body.tune[k];
    Object.assign(curTune, tune);
    resetCtx = { seed: s, tune: Object.assign({}, curTune), curriculum: { on: curriculum.on, floor: curriculum.floor } };
    journal = []; markLen = null;
    await _doStart(s, curTune, curriculum);
    return { status: 200, body: _frame() };
  }
  async function _step(body) {
    const keys = (body && typeof body.keys === "string") ? body.keys : "";
    if (!keys) return { status: 400, body: { error: "no keys" } };
    await _rawStepKeys(keys);
    journal.push({ t: "step", keys });
    return { status: 200, body: _frame() };
  }
  async function _live(body) {
    const name = body && body.name; const val = +(body && body.value);
    if (!(name in nameIdx)) return { status: 400, body: { error: "unknown knob" } };
    curTune[name] = val; _rawLive(name, val);
    await F.step(obs, 18); await _settle();
    journal.push({ t: "live", name, val });
    return { status: 200, body: _frame() };
  }
  async function _modify(body) {
    const changes = (body && body.changes) || {};
    const clean = {}; for (const k in changes) clean[k] = changes[k] | 0;
    await _rawModify(clean);
    journal.push({ t: "modify", changes: clean });
    return { status: 200, body: _frame() };
  }
  async function _undo(body) {
    let n = (body && Number.isFinite(+body.n)) ? (+body.n | 0) : 1;
    const stepIdx = []; journal.forEach((e, i) => { if (e.t === "step") stepIdx.push(i); });
    if (!stepIdx.length) return { status: 400, body: { error: "nothing to undo" } };
    n = Math.max(1, Math.min(n, stepIdx.length));
    const cut = stepIdx[stepIdx.length - n];
    await _replayTo(cut);
    return { status: 200, body: Object.assign(_frame(), { undos_left: journal.reduce((a, e) => a + (e.t === "step" ? 1 : 0), 0) }) };
  }
  function _mark() { markLen = journal.length; return { status: 200, body: { marked: true } }; }
  async function _restoreMark() {
    if (markLen == null) return { status: 400, body: { error: "no checkpoint set" } };
    await _replayTo(markLen);
    return { status: 200, body: _frame() };
  }

  /* ---- serialize every engine-mutating handler (C engine is not reentrant) ---- */
  let q = Promise.resolve();
  function enqueue(fn) { const run = q.then(fn, fn); q = run.catch(() => {}); return run; }

  async function dispatch(path, method, body) {
    await ready;
    if (path === "/catalog") return _catalog();
    if (path === "/current") return { status: 200, body: { live: false } };           // no server-side resume in the browser
    if (path === "/gifs") return { status: 200, body: [] };
    if (path === "/reset") return enqueue(() => _reset(body));
    if (path === "/step") return enqueue(() => _step(body));
    if (path === "/live") return enqueue(() => _live(body));
    if (path === "/modify") return enqueue(() => _modify(body));
    if (path === "/undo") return enqueue(() => _undo(body));
    if (path === "/mark") return enqueue(() => _mark());
    if (path === "/restore_mark") return enqueue(() => _restoreMark());
    if (path === "/record_start" || path === "/record_stop")
      return { status: 400, body: { error: "recording is only available in the local app" } };
    return null; // not ours
  }

  /* ---- install the fetch shim ---- */
  const origFetch = (typeof window !== "undefined" && window.fetch) ? window.fetch.bind(window) : null;
  async function shimFetch(url, opts) {
    let path;
    try { path = new URL(url, (typeof location !== "undefined") ? location.href : "http://x/").pathname; }
    catch (e) { path = String(url).split("?")[0]; }
    let body = null;
    if (opts && opts.body) { try { body = JSON.parse(opts.body); } catch (e) { body = null; } }
    const method = (opts && opts.method) || "GET";
    const res = await dispatch(path, method, body);
    if (res == null) { if (origFetch) return origFetch(url, opts); throw new Error("no backend for " + path); }
    return new Response(JSON.stringify(res.body), { status: res.status, headers: { "Content-Type": "application/json" } });
  }
  if (typeof window !== "undefined") window.fetch = shimFetch;

  /* ---- public API for the page's six-level curriculum toggle ---- */
  const NHConsole = {
    ready,
    setCurriculum(on, floor) { curriculum.on = !!on; if (floor) curriculum.floor = floor | 0; },
    getCurriculum() { return { on: curriculum.on, floor: curriculum.floor }; },
    curriculumName(floor) { const N = { 1: "DoD 1", 2: "DoD 2", 3: "DoD 3", 4: "Gehennom 48", 5: "Gehennom 49", 6: "Gehennom 50" }; return N[floor] || ("floor " + floor); },
  };
  if (typeof window !== "undefined") window.NHConsole = NHConsole;
  if (typeof module !== "undefined" && module.exports) module.exports = { dispatch, NHConsole, ready };
})();
