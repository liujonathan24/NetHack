/* nethack_web.js — browser driver for the WASM NetHack engine (works in Node too).
 * Wraps the nleweb_* API: reset / step / gotoFloor (curriculum) / render.
 * Fiber-swapping calls (start/step/gotoAbs) are Asyncify-async, so they return
 * Promises and must be awaited; the live ctx is read from nleweb_ctx (the value
 * returned across the JS boundary from an unwound call is lost). */
(function (root) {
  const CURR_NAMES = {1:"DoD 1",2:"DoD 2",3:"DoD 3",4:"Gehennom 48",5:"Gehennom 49",6:"Gehennom 50"};
  const BASE_OPTS = "autopickup,color,disclose:+i +a +v +g +c +o,mention_walls,nobones," +
                    "nocmdassist,nolegacy,nosparkle,pickup_burden:unencumbered," +
                    "pickup_types:$?!/,runmode:teleport,showexp,showscore,time";

  class NetHackGame {
    constructor(Module) {
      const M = this.M = Module;
      const cw = (n, r, a, o) => M.cwrap(n, r, a, o);
      this._start   = cw('nleweb_start', 'number', ['number','number','string'], {async:true});
      this._step    = cw('nleweb_step', 'number', ['number','number'], {async:true});
      this._gotoAbs = cw('nleweb_goto_abs', 'number', ['number','number'], {async:true});
      this._newObs  = cw('nleweb_new_obs', 'number', []);
      this._ttyChars= cw('nleweb_tty_chars', 'number', ['number']);
      this._colors  = cw('nleweb_tty_colors', 'number', ['number']);
      this._blstats = cw('nleweb_blstats', 'number', ['number']);
      this._msgPtr  = cw('nleweb_message', 'number', ['number']);
      this._done    = cw('nleweb_done', 'number', ['number']);
      this._inGame  = cw('nleweb_in_game', 'number', ['number']);
      this._onStair = cw('nleweb_hero_on_stair', 'number', []);
      this._numDgn  = cw('nleweb_num_dungeons', 'number', []);
      this._dgnInfo = cw('nleweb_dungeon_info', 'number', ['number','number','number','number','number']);
      this._tuneCount = cw('nleweb_tune_count', 'number', []);
      this._tuneName  = cw('nleweb_tune_name', 'number', ['number']);
      this._setTune   = cw('nleweb_set_tune', null, ['number','number']);
      this._clearTune = cw('nleweb_clear_tune', null, []);
      this.TR = cw('nleweb_tty_rows','number',[])();
      this.TC = cw('nleweb_tty_cols','number',[])();
      this.obs = 0;
      // deterministic snapshot/undo: replay (seed, tune, startFloor, actions).
      this._log = { seed: 19, character: null, tune: {}, startFloor: null, actions: [] };
    }

    /* ---- difficulty knobs (applied at the next reset, generation-time) ---- */
    tuneCatalog() {
      const n = this._tuneCount(), out = [];
      for (let i = 0; i < n; i++) out.push(this.M.UTF8ToString(this._tuneName(i)));
      return out;
    }
    _tuneIndex(name) { return this.tuneCatalog().indexOf(name); }
    setTune(name, val) {           // queue a knob; takes effect on next reset()
      const i = this._tuneIndex(name);
      if (i < 0) throw new Error("unknown knob: " + name);
      this._log.tune[name] = val;
    }
    clearTune() { this._log.tune = {}; }
    _applyTune() {                 // push queued knobs into the engine pre-start
      this._clearTune();
      for (const [name, val] of Object.entries(this._log.tune)) {
        const i = this._tuneIndex(name);
        if (i >= 0) this._setTune(i, val);
      }
    }
    _opts(character) { return BASE_OPTS + ",name:Agent-" + (character || "Val-hum-neu-fem"); }

    /* Boot a fresh game from the current _log's seed/character/tune. Shared by
       reset() and by deterministic replay (undo/restore). Does NOT touch the log. */
    async _boot() {
      this._applyTune();
      if (!this.obs) this.obs = this._newObs();
      await this._start(this.obs, this._log.seed >>> 0, this._opts(this._log.character));
      await this._settle();
    }
    async reset(seed = 19, character) {
      this._log = { seed: seed >>> 0, character: character || null,
                    tune: this._log.tune, startFloor: null, actions: [] };
      await this._boot();
      return this.state();
    }
    async _settle() { for (let i = 0; i < 4; i++) await this._step(this.obs, 27); } // dismiss --More--
    async step(key) {
      this._log.actions.push(key | 0);
      await this._step(this.obs, key | 0);
      return this.state();
    }

    /* ---- deterministic snapshot / undo (replay seed + action log) ---- */
    async undo() {                 // step back one action by replaying all-but-last
      if (!this._log.actions.length) return this.state();
      this._log.actions.pop();
      return this._replay();
    }
    async _replay() {              // rebuild the exact game from the log
      const acts = this._log.actions;
      this._log.actions = [];
      await this._boot();
      if (this._log.startFloor) await this._gotoFloorInternal(this._log.startFloor);
      for (const k of acts) { this._log.actions.push(k); await this._step(this.obs, k | 0); }
      return this.state();
    }
    snapshot() { return JSON.stringify(this._log); }  // tiny, shareable game state
    async restore(json) {          // load a snapshot string and rebuild the game
      const L = (typeof json === 'string') ? JSON.parse(json) : json;
      this._log = { seed: L.seed >>> 0, character: L.character || null,
                    tune: L.tune || {}, startFloor: L.startFloor || null,
                    actions: [] };
      const acts = L.actions || [];
      await this._boot();
      if (this._log.startFloor) await this._gotoFloorInternal(this._log.startFloor);
      for (const k of acts) { this._log.actions.push(k); await this._step(this.obs, k | 0); }
      return this.state();
    }

    dungeonTable() {
      const M = this.M, n = this._numDgn(), t = [];
      const nm = M._malloc(24), ds = M._malloc(4), nu = M._malloc(4);
      for (let i = 0; i < n; i++) {
        this._dgnInfo(i, nm, 24, ds, nu);
        t.push({ dnum: i, name: M.UTF8ToString(nm),
                 depth_start: M.HEAP32[ds>>2], num: M.HEAP32[nu>>2] });
      }
      M._free(nm); M._free(ds); M._free(nu);
      return t;
    }
    /* Curriculum: place the hero on floor 1..6 (DoD 1-3 / Gehennom 48-50).
       Records startFloor so undo/restore reproduce the same starting position. */
    async gotoFloor(floor) {
      this._log.startFloor = floor;
      this._log.actions = [];              // curriculum jump resets the action log
      await this._gotoFloorInternal(floor);
      return this.state();
    }
    async _gotoFloorInternal(floor) {      // the raw jump, no logging (used by replay)
      const t = this.dungeonTable();
      const dod = t.find(d => /Dungeons of Doom/.test(d.name));
      const geh = t.find(d => /Gehennom/.test(d.name));
      let dnum, dlevel;
      if (floor <= 3) { dnum = dod.dnum; dlevel = floor; }
      else { dnum = geh.dnum; dlevel = (44 + floor) - geh.depth_start + 1; } // 48+(floor-4)
      await this._gotoAbs(dnum, dlevel);   // schedules; process + render on next step
      await this._step(this.obs, 27);
    }

    ttyRows() {
      const M = this.M, p = this._ttyChars(this.obs), rows = [];
      for (let r = 0; r < this.TR; r++) {
        let s = '';
        for (let c = 0; c < this.TC; c++) {
          const ch = M.HEAPU8[p + r*this.TC + c];
          s += (ch >= 32 && ch < 127) ? String.fromCharCode(ch) : ' ';
        }
        rows.push(s);
      }
      return rows;
    }
    bl() {
      const M = this.M, p = this._blstats(this.obs) >> 2;
      return { x:M.HEAP32[p], y:M.HEAP32[p+1], hp:M.HEAP32[p+10], maxhp:M.HEAP32[p+11],
               depth:M.HEAP32[p+12], gold:M.HEAP32[p+13], ac:M.HEAP32[p+16],
               xp:M.HEAP32[p+18], dnum:M.HEAP32[p+23] };
    }
    curriculumFloor() {
      const b = this.bl(), t = this.dungeonTable();
      const dod = t.find(d => /Dungeons of Doom/.test(d.name));
      const geh = t.find(d => /Gehennom/.test(d.name));
      if (dod && b.dnum === dod.dnum && b.depth >= 1 && b.depth <= 3) return b.depth;
      if (geh && b.dnum === geh.dnum && b.depth >= 48) return 3 + (b.depth - 48 + 1);
      return 0;
    }
    message() { return this.M.UTF8ToString(this._msgPtr(this.obs)); }
    state() {
      const fl = this.curriculumFloor();
      return { tty: this.ttyRows(), bl: this.bl(), message: this.message(),
               done: !!this._done(this.obs), inGame: !!this._inGame(this.obs),
               onStair: this._onStair(), curriculumFloor: fl,
               curriculumName: CURR_NAMES[fl] || "off-path" };
    }
  }
  root.NetHackGame = NetHackGame;
  if (typeof module !== 'undefined' && module.exports) module.exports = { NetHackGame };
})(typeof window !== 'undefined' ? window : globalThis);
