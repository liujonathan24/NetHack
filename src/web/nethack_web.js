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
      this.TR = cw('nleweb_tty_rows','number',[])();
      this.TC = cw('nleweb_tty_cols','number',[])();
      this.obs = 0;
    }
    _opts(character) { return BASE_OPTS + ",name:Agent-" + (character || "Val-hum-neu-fem"); }

    async reset(seed = 19, character) {
      this.obs = this._newObs();
      await this._start(this.obs, seed >>> 0, this._opts(character));
      await this._settle();
      return this.state();
    }
    async _settle() { for (let i = 0; i < 4; i++) await this._step(this.obs, 27); } // dismiss --More--
    async step(key) { await this._step(this.obs, key | 0); return this.state(); }

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
    /* Curriculum: place the hero on floor 1..6 (DoD 1-3 / Gehennom 48-50). */
    async gotoFloor(floor) {
      const t = this.dungeonTable();
      const dod = t.find(d => /Dungeons of Doom/.test(d.name));
      const geh = t.find(d => /Gehennom/.test(d.name));
      let dnum, dlevel;
      if (floor <= 3) { dnum = dod.dnum; dlevel = floor; }
      else { dnum = geh.dnum; dlevel = (44 + floor) - geh.depth_start + 1; } // 48+(floor-4)
      await this._gotoAbs(dnum, dlevel);   // schedules; process + render on next step
      await this._step(this.obs, 27);
      return this.state();
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
