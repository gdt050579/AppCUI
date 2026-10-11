// @ts-check
// AppCUI BrowserTerminal host: HTML5 canvas renderer + DOM input bridge.
// Pinned to BROWSER_ABI_VERSION = 1 (AppCUI/src/Terminal/BrowserTerminal/BrowserTerminal.hpp).
//
// Hot paths (draw / input handlers) allocate nothing per frame or per event: cells are diffed against a shadow copy,
// glyph strings and colors come from pre-built tables, and wasm memory views are only rebuilt when the screen buffer
// moves (resize) or the wasm memory grows.

/**
 * @typedef {object} AppCUIModule
 * @property {Uint32Array} HEAPU32
 * @property {() => number} _appcui_browser_abi_version
 * @property {() => number} _appcui_browser_frame_header
 * @property {(key: number, unicodeChar: number) => number} _appcui_browser_key
 * @property {(modifiers: number) => number} _appcui_browser_shift_state
 * @property {(type: number, x: number, y: number, buttons: number, modifiers: number) => number} _appcui_browser_mouse
 * @property {(x: number, y: number, direction: number, modifiers: number) => number} _appcui_browser_wheel
 * @property {(columns: number, rows: number) => number} _appcui_browser_resize
 * @property {() => number} _appcui_browser_close
 * @property {() => number} _appcui_browser_redraw
 */

/**
 * @typedef {object} AppCUIHostOptions
 * @property {string} fontFamily    e.g. '"JuliaMono", monospace'
 * @property {number} fontSizeCss   CSS pixels
 * @property {number} [lineHeight]  multiplier of ascent + descent (default 1.0)
 * @property {number} [maxColumns]  must not exceed BROWSER_MAX_COLUMNS (1024)
 * @property {number} [maxRows]     must not exceed BROWSER_MAX_ROWS (512)
 * @property {(host: AppCUIHost) => void} [onExit]  called once the application has exited; by default the page shows
 *                                                   an "application has exited" overlay with a Restart button
 */

const ABI_MAGIC = 0x49554341;
const ABI_VERSION = 1;
// BrowserFrameHeader word indexes
const H_MAGIC = 0, H_ABI = 1, H_FRAME_SEQ = 2, H_LAYOUT_SEQ = 3, H_CELLS_PTR = 4, H_WIDTH = 5, H_HEIGHT = 6,
  H_CURSOR_X = 7, H_CURSOR_Y = 8, H_CURSOR_VISIBLE = 9;

const MOD_ALT = 0x1000, MOD_CTRL = 0x2000, MOD_SHIFT = 0x4000;
const MOUSE_DOWN = 0, MOUSE_UP = 1, MOUSE_MOVE = 2;
const BTN_LEFT = 0x01, BTN_CENTER = 0x02, BTN_RIGHT = 0x04, BTN_DOUBLE = 0x08;
const DOUBLE_CLICK_MS = 400;

// Graphics::Color 0..15 (same RGB values as SDLTerminalScreen.cpp)
const PALETTE = [
  '#000000', '#000080', '#13a10e', '#3a96dd', '#c50f1f', '#881798', '#c19c00', '#cccccc',
  '#767676', '#3b78ff', '#16c60c', '#61d6d6', '#e74856', '#b4009e', '#f9f1a5', '#f2f2f2',
];
// 256 entries: any byte value is safe; Transparent (16) and garbage fall back to Silver on Black
const FG = new Array(256).fill(PALETTE[7]);
const BG = new Array(256).fill(PALETTE[0]);
for (let i = 0; i < 16; i++) { FG[i] = PALETTE[i]; BG[i] = PALETTE[i]; }

// KeyboardEvent.code -> Input::Key (pinned by the static_asserts in BrowserExports.cpp)
/** @type {Record<string, number>} */
const KEY = (() => {
  /** @type {Record<string, number>} */
  const k = {
    Enter: 13, NumpadEnter: 13, Escape: 14, Insert: 15, Delete: 16, Backspace: 17, Tab: 18,
    ArrowLeft: 19, ArrowUp: 20, ArrowDown: 21, ArrowRight: 22, PageUp: 23, PageDown: 24,
    Home: 25, End: 26, Space: 27,
  };
  for (let i = 1; i <= 12; i++) k['F' + i] = i;
  for (let i = 0; i < 26; i++) k['Key' + String.fromCharCode(65 + i)] = 28 + i;
  for (let i = 0; i < 10; i++) { k['Digit' + i] = 54 + i; k['Numpad' + i] = 54 + i; }
  return k;
})();
const MODIFIER_CODES = new Set(['ShiftLeft', 'ShiftRight', 'ControlLeft', 'ControlRight', 'AltLeft', 'AltRight']);

// Box drawing U+2500..U+257F drawn with rectangles. Arms: up = 1, right = 2, down = 4, left = 8; the low nibble
// holds single (light) arms, the high nibble double arms. 0 = mixed / rare glyphs -> rendered with the font.
const BOX = new Uint8Array(0x80);
BOX[0x00] = 10; BOX[0x02] = 5; BOX[0x0c] = 6; BOX[0x10] = 12; BOX[0x14] = 3; BOX[0x18] = 9;  // ─ │ ┌ ┐ └ ┘
BOX[0x1c] = 7; BOX[0x24] = 13; BOX[0x2c] = 14; BOX[0x34] = 11; BOX[0x3c] = 15;               // ├ ┤ ┬ ┴ ┼
BOX[0x50] = 10 << 4; BOX[0x51] = 5 << 4; BOX[0x54] = 6 << 4; BOX[0x57] = 12 << 4;            // ═ ║ ╔ ╗
BOX[0x5a] = 3 << 4; BOX[0x5d] = 9 << 4; BOX[0x60] = 7 << 4; BOX[0x63] = 13 << 4;             // ╚ ╝ ╠ ╣
BOX[0x66] = 14 << 4; BOX[0x69] = 11 << 4; BOX[0x6c] = 15 << 4;                                // ╦ ╩ ╬

// Code points without a visible glyph (C0 controls, DEL, C1 controls) are shown as their code page 437 symbols, the same
// mapping the SDL frontend uses (SDLTerminalScreen.cpp); printable code points keep their Unicode meaning.
const CONTROL_GLYPHS = new Uint16Array(0xa0);
[0x263a, 0x263b, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022, 0x25d8, 0x25cb, 0x25d9, 0x2642, 0x2640, 0x266a, 0x266b,
  0x263c, 0x25ba, 0x25c4, 0x2195, 0x203c, 0xb6, 0xa7, 0x25ac, 0x21a8, 0x2191, 0x2193, 0x2192, 0x2190, 0x221f, 0x2194,
  0x25b2, 0x25bc].forEach((g, i) => { CONTROL_GLYPHS[0x01 + i] = g; });
CONTROL_GLYPHS[0x7f] = 0x2302;
[0xc7, 0xfc, 0xe9, 0xe2, 0xe4, 0xe0, 0xe5, 0xe7, 0xea, 0xeb, 0xe8, 0xef, 0xee, 0xec, 0xc4, 0xc5, 0xc9, 0xe6, 0xc6,
  0xf4, 0xf6, 0xf2, 0xfb, 0xf9, 0xff, 0xd6, 0xdc, 0xa2, 0xa3, 0xa5, 0x20a7, 0x192].forEach((g, i) => { CONTROL_GLYPHS[0x80 + i] = g; });

export class AppCUIHost {
  /**
   * @param {HTMLCanvasElement} canvas
   * @param {HTMLElement} container   the element whose size decides the number of columns / rows
   * @param {AppCUIHostOptions} options
   */
  constructor(canvas, container, options) {
    this.canvas = canvas;
    this.container = container;
    this.opts = { lineHeight: 1.0, maxColumns: 1024, maxRows: 512, ...options };
    const ctx = canvas.getContext('2d', { alpha: false });
    if (!ctx) throw new Error('2D canvas context not available');
    this.ctx = ctx;

    /** @type {AppCUIModule | null} */
    this.mod = null;
    // metrics (device pixels, integers)
    this.dpr = 1; this.cellW = 8; this.cellH = 16; this.baseline = 12; this.lineT = 1; this.doubleGap = 2; this.fontCss = '';
    // bound state
    this.hdr = 0;
    /** @type {ArrayBufferLike | null} */
    this.boundBuffer = null;
    this.boundLayoutSeq = -1;
    this.cells = new Uint32Array(0);
    this.shadow = new Uint32Array(0);
    this.cols = 0; this.rows = 0;
    this.forceFull = true;
    this.curX = -1; this.curY = -1; this.curVisible = false;
    this.lastFill = '';
    /** @type {(string | undefined)[]} */
    this.glyphs = new Array(65536);
    // scheduling
    this.drawQueued = false;
    this.drawBound = () => this.draw();
    /** @type {(() => void) | null} */
    this.wakeResolve = null;
    this.wakeTimer = 0;
    this.exited = false;
    // input state
    this.mods = 0;
    this.lastCellX = -1; this.lastCellY = -1; this.lastButtons = -1;
    this.lastDownTime = 0; this.lastDownX = -1; this.lastDownY = -1;
    this.gridCols = 0; this.gridRows = 0;

    this.measure();
    this.computeGrid();
    this.installInput();
    new ResizeObserver(() => this.onContainerResize()).observe(container);
    this.watchDpr();
  }

  // ------------------------------------------------------------------------------------------ called by C++ / boot
  /** @param {AppCUIModule} mod */
  attach(mod) {
    if (mod._appcui_browser_abi_version() !== ABI_VERSION) throw new Error('AppCUI browser ABI mismatch');
    this.mod = mod;
    this.hdr = (mod._appcui_browser_frame_header() >>> 0) >>> 2;
  }
  /** @returns {number} columns | rows << 16 */
  initialSize() {
    return (this.gridCols & 0xffff) | ((this.gridRows & 0xffff) << 16);
  }
  /** @param {number} _headerPtr */
  present(_headerPtr) {
    if (this.drawQueued || this.exited) return;
    this.drawQueued = true;
    queueMicrotask(this.drawBound); // every flush of the current task -> one draw
  }
  /**
   * Idle path only (a modal loop with nothing to do): resolved by input or by the timeout.
   * @param {number} timeoutMs
   * @returns {Promise<void>}
   */
  waitForInput(timeoutMs) {
    return new Promise((resolve) => {
      this.wakeResolve = resolve;
      this.wakeTimer = window.setTimeout(() => this.wake(), timeoutMs);
    });
  }
  onExit() {
    if (this.exited) return;
    this.draw(); // the final frame: a draw queued for this task would otherwise be skipped once `exited` is set
    this.exited = true;
    this.wake();
    // the canvas keeps the last frame: without feedback a closed application looks frozen
    if (this.opts.onExit) this.opts.onExit(this);
    else this.showExitOverlay();
  }

  /** Default exit feedback: dims the last frame and offers to restart the application (reloads the page). */
  showExitOverlay() {
    const container = this.container;
    if (getComputedStyle(container).position === 'static') container.style.position = 'relative';
    const overlay = document.createElement('div');
    overlay.className = 'appcui-exit-overlay';
    overlay.style.cssText = 'position:absolute;inset:0;display:flex;align-items:center;justify-content:center;' +
      'background:rgba(0,0,0,0.65);font:16px ' + this.opts.fontFamily + ';color:' + PALETTE[15];
    const panel = document.createElement('div');
    panel.style.cssText = 'background:' + PALETTE[1] + ';border:3px double ' + PALETTE[15] + ';padding:20px 32px;' +
      'text-align:center;line-height:1.6';
    const text = document.createElement('div');
    text.textContent = 'The application has exited.';
    const restart = document.createElement('button');
    restart.type = 'button';
    restart.textContent = 'Restart';
    restart.style.cssText = 'margin-top:14px;padding:4px 24px;font:inherit;color:#000;background:' + PALETTE[15] +
      ';border:0;cursor:pointer';
    restart.addEventListener('click', () => location.reload());
    panel.append(text, restart);
    overlay.append(panel);
    container.append(overlay);
    restart.focus(); // Enter / Space restarts
  }

  // ------------------------------------------------------------------------------------------ rendering
  draw() {
    this.drawQueued = false;
    const mod = this.mod;
    if (!mod || this.exited) return;
    const heap = mod.HEAPU32; // re-read every time: replaced when the wasm memory grows
    const h = this.hdr;
    if (heap[h + H_MAGIC] !== ABI_MAGIC || heap[h + H_ABI] !== ABI_VERSION) return;

    const layoutSeq = heap[h + H_LAYOUT_SEQ];
    if (layoutSeq !== this.boundLayoutSeq || heap.buffer !== this.boundBuffer) this.rebind(heap, layoutSeq);

    // a moved / hidden cursor -> the cell under the old position must be repainted
    const cx = heap[h + H_CURSOR_X], cy = heap[h + H_CURSOR_Y], cv = heap[h + H_CURSOR_VISIBLE] !== 0;
    if (cx !== this.curX || cy !== this.curY || cv !== this.curVisible) {
      this.invalidateCell(this.curX, this.curY);
      this.curX = cx; this.curY = cy; this.curVisible = cv;
      this.invalidateCell(cx, cy);
    }
    this.paintCells();
    if (cv && cx < this.cols && cy < this.rows) this.paintCursor(cx, cy);
  }

  /**
   * Cold path: the screen buffer moved (resize) or the wasm memory grew.
   * @param {Uint32Array} heap
   * @param {number} layoutSeq
   */
  rebind(heap, layoutSeq) {
    const h = this.hdr;
    const ptr = heap[h + H_CELLS_PTR] >>> 0;
    const w = heap[h + H_WIDTH] >>> 0, r = heap[h + H_HEIGHT] >>> 0;
    if ((ptr & 3) !== 0 || w === 0 || r === 0 || w > this.opts.maxColumns || r > this.opts.maxRows ||
        (ptr >>> 2) + w * r > heap.length) {
      throw new Error('AppCUI: invalid frame header');
    }
    this.cells = heap.subarray(ptr >>> 2, (ptr >>> 2) + w * r);
    if (w !== this.cols || r !== this.rows || this.shadow.length !== w * r) {
      this.cols = w; this.rows = r;
      this.shadow = new Uint32Array(w * r);
      this.canvas.width = w * this.cellW; // resets the 2D context state
      this.canvas.height = r * this.cellH;
      this.canvas.style.width = (w * this.cellW) / this.dpr + 'px';
      this.canvas.style.height = (r * this.cellH) / this.dpr + 'px';
      this.applyContextState();
    }
    this.boundBuffer = heap.buffer;
    this.boundLayoutSeq = layoutSeq;
    this.forceFull = true;
  }

  paintCells() {
    const cells = this.cells, shadow = this.shadow, ctx = this.ctx;
    const cols = this.cols, rows = this.rows, cw = this.cellW, ch = this.cellH;
    const full = this.forceFull;
    this.forceFull = false;
    for (let y = 0; y < rows; y++) {
      const row = y * cols;
      const py = y * ch;
      let x = 0;
      while (x < cols) {
        const v = cells[row + x];
        if (!full && v === shadow[row + x]) { x++; continue; }
        // maximal run of changed cells that share the same background -> one fillRect
        const bg = v >>> 24;
        let end = x + 1;
        while (end < cols) {
          const w = cells[row + end];
          if ((!full && w === shadow[row + end]) || (w >>> 24) !== bg) break;
          end++;
        }
        this.setFill(BG[bg]);
        ctx.fillRect(x * cw, py, (end - x) * cw, ch);
        for (let i = x; i < end; i++) {
          const c = cells[row + i];
          shadow[row + i] = c;
          const code = c & 0xffff;
          if (code === 0x20 || code === 0) continue;
          const fg = (c >>> 16) & 0xff;
          if (!this.drawProcedural(code, FG[fg], i * cw, py)) {
            this.setFill(FG[fg]);
            ctx.fillText(this.glyph(code), i * cw, py + this.baseline, cw); // maxWidth: never bleed into neighbours
          }
        }
        x = end;
      }
    }
  }

  /** @param {number} cx @param {number} cy */
  paintCursor(cx, cy) {
    const c = this.cells[cy * this.cols + cx];
    this.setFill(FG[(c >>> 16) & 0xff]);
    const t = Math.max(2, this.lineT * 2);
    this.ctx.fillRect(cx * this.cellW, (cy + 1) * this.cellH - t, this.cellW, t);
  }

  /** @param {number} x @param {number} y */
  invalidateCell(x, y) {
    if (x >= 0 && y >= 0 && x < this.cols && y < this.rows) {
      const i = y * this.cols + x;
      this.shadow[i] = ~this.cells[i] >>> 0; // guaranteed different -> repainted by the next paintCells
    }
  }

  /**
   * Block elements and light lines are drawn with rectangles: no gaps between cells at any zoom / dpr.
   * @param {number} code @param {string} color @param {number} x @param {number} y
   * @returns {boolean} true when the glyph was drawn
   */
  drawProcedural(code, color, x, y) {
    const ctx = this.ctx, cw = this.cellW, ch = this.cellH;
    if (code >= 0x2580 && code <= 0x259f) {
      const hw = cw >> 1, hh = ch >> 1;
      switch (code) {
        case 0x2588: this.setFill(color); ctx.fillRect(x, y, cw, ch); return true;           // full block
        case 0x2580: this.setFill(color); ctx.fillRect(x, y, cw, hh); return true;           // upper half
        case 0x2584: this.setFill(color); ctx.fillRect(x, y + hh, cw, ch - hh); return true; // lower half
        case 0x258c: this.setFill(color); ctx.fillRect(x, y, hw, ch); return true;           // left half
        case 0x2590: this.setFill(color); ctx.fillRect(x + hw, y, cw - hw, ch); return true; // right half
        case 0x2591: case 0x2592: case 0x2593: {                                             // light / medium / dark shade
          this.setFill(color);
          ctx.globalAlpha = code === 0x2591 ? 0.25 : code === 0x2592 ? 0.5 : 0.75;
          ctx.fillRect(x, y, cw, ch);
          ctx.globalAlpha = 1;
          return true;
        }
        default: return false;
      }
    }
    if (code >= 0x2500 && code <= 0x257f) {
      const arms = BOX[code - 0x2500];
      if (arms === 0) return false;
      const t = this.lineT;
      const mx = x + ((cw - t) >> 1), my = y + ((ch - t) >> 1);
      this.setFill(color);
      if (arms < 16) {
        if (arms & 1) ctx.fillRect(mx, y, t, my - y + t);
        if (arms & 4) ctx.fillRect(mx, my, t, y + ch - my);
        if (arms & 8) ctx.fillRect(x, my, mx - x + t, t);
        if (arms & 2) ctx.fillRect(mx, my, x + cw - mx, t);
      } else {
        this.drawDoubleLines(arms >> 4, x, y, mx, my);
      }
      return true;
    }
    return false;
  }

  /**
   * Two parallel strokes per arm. The outer / inner stroke of each side is interrupted where the perpendicular arm
   * enters the cell, which yields correct corners and tees (╔ ╠ ╬ ...) for every combination of arms.
   * @param {number} arms up = 1, right = 2, down = 4, left = 8
   * @param {number} x @param {number} y @param {number} mx @param {number} my  cell origin / single-line center
   */
  drawDoubleLines(arms, x, y, mx, my) {
    const ctx = this.ctx, cw = this.cellW, ch = this.cellH, t = this.lineT, g = this.doubleGap;
    const up = (arms & 1) !== 0, right = (arms & 2) !== 0, down = (arms & 4) !== 0, left = (arms & 8) !== 0;
    const x1 = mx - g, x2 = mx + g, y1 = my - g, y2 = my + g;
    const right1 = x + cw, bottom1 = y + ch;
    // horizontal strokes (y1 = upper, y2 = lower): left part, right part, and the middle unless an arm enters there
    if (left || right) {
      if (left) { ctx.fillRect(x, y1, x1 - x + t, t); ctx.fillRect(x, y2, x1 - x + t, t); }
      if (right) { ctx.fillRect(x2, y1, right1 - x2, t); ctx.fillRect(x2, y2, right1 - x2, t); }
      if (!up) ctx.fillRect(x1, y1, x2 - x1 + t, t);
      if (!down) ctx.fillRect(x1, y2, x2 - x1 + t, t);
    }
    // vertical strokes (x1 = left, x2 = right)
    if (up || down) {
      if (up) { ctx.fillRect(x1, y, t, y1 - y + t); ctx.fillRect(x2, y, t, y1 - y + t); }
      if (down) { ctx.fillRect(x1, y2, t, bottom1 - y2); ctx.fillRect(x2, y2, t, bottom1 - y2); }
      if (!left) ctx.fillRect(x1, y1, t, y2 - y1 + t);
      if (!right) ctx.fillRect(x2, y1, t, y2 - y1 + t);
    }
  }

  /** @param {number} code */
  glyph(code) {
    let s = this.glyphs[code];
    if (s === undefined) {
      const shown = code < 0xa0 && CONTROL_GLYPHS[code] !== 0 ? CONTROL_GLYPHS[code] : code;
      s = shown >= 0xd800 && shown <= 0xdfff ? '�' : String.fromCharCode(shown);
      this.glyphs[code] = s; // built once per distinct code point
    }
    return s;
  }

  /** @param {string} style */
  setFill(style) {
    if (style !== this.lastFill) { this.ctx.fillStyle = style; this.lastFill = style; }
  }

  // ------------------------------------------------------------------------------------------ metrics, resize, DPR
  measure() {
    this.dpr = window.devicePixelRatio || 1;
    const px = Math.max(6, Math.round(this.opts.fontSizeCss * this.dpr));
    this.fontCss = px + 'px ' + this.opts.fontFamily;
    this.ctx.font = this.fontCss;
    const m = this.ctx.measureText('M');
    const asc = Math.ceil(m.fontBoundingBoxAscent || px * 0.8);
    const desc = Math.ceil(m.fontBoundingBoxDescent || px * 0.2);
    this.cellW = Math.max(1, Math.ceil(m.width));
    this.cellH = Math.max(1, Math.ceil((asc + desc) * this.opts.lineHeight));
    this.baseline = asc + ((this.cellH - asc - desc) >> 1);
    this.lineT = Math.max(1, Math.round(this.dpr));
    // distance between the center of a single line and each stroke of a double line (>= 1 px of gap)
    this.doubleGap = Math.max(this.lineT + 1, Math.round(this.cellW / 5));
  }
  applyContextState() {
    this.ctx.font = this.fontCss;
    this.ctx.textBaseline = 'alphabetic';
    this.ctx.textAlign = 'left';
    this.lastFill = '';
  }
  /** @returns {boolean} true when the grid size changed */
  computeGrid() {
    const r = this.container.getBoundingClientRect();
    const cols = Math.min(this.opts.maxColumns, Math.max(1, Math.floor((r.width * this.dpr) / this.cellW)));
    const rows = Math.min(this.opts.maxRows, Math.max(1, Math.floor((r.height * this.dpr) / this.cellH)));
    const changed = cols !== this.gridCols || rows !== this.gridRows;
    this.gridCols = cols; this.gridRows = rows;
    return changed;
  }
  onContainerResize() {
    if (this.computeGrid() && this.mod && !this.exited) {
      this.mod._appcui_browser_resize(this.gridCols, this.gridRows);
      this.wake();
    }
  }
  watchDpr() {
    const mq = window.matchMedia('(resolution: ' + this.dpr + 'dppx)');
    mq.addEventListener('change', () => {
      this.measure();
      this.cols = 0; this.rows = 0; this.boundLayoutSeq = -1; // force a rebind with the new cell size
      this.onContainerResize();
      if (this.mod && !this.exited) { this.mod._appcui_browser_redraw(); this.wake(); }
      this.watchDpr();
    }, { once: true });
  }

  // ------------------------------------------------------------------------------------------ input
  wake() {
    if (this.wakeTimer) { clearTimeout(this.wakeTimer); this.wakeTimer = 0; }
    const r = this.wakeResolve;
    if (r) { this.wakeResolve = null; r(); }
  }
  /** @param {KeyboardEvent | MouseEvent} e */
  modsOf(e) {
    return (e.altKey ? MOD_ALT : 0) | (e.ctrlKey ? MOD_CTRL : 0) | (e.shiftKey ? MOD_SHIFT : 0);
  }
  /** @param {MouseEvent} e */
  cellOf(e) {
    this.lastCellX = Math.floor((e.offsetX * this.dpr) / this.cellW) | 0;
    this.lastCellY = Math.floor((e.offsetY * this.dpr) / this.cellH) | 0;
  }
  /** DOM buttons bitmask (1 left, 2 right, 4 middle) -> Input::MouseButton @param {number} b */
  static buttonsOf(b) {
    return ((b & 1) ? BTN_LEFT : 0) | ((b & 4) ? BTN_CENTER : 0) | ((b & 2) ? BTN_RIGHT : 0);
  }
  /** DOM button index (0 left, 1 middle, 2 right) -> Input::MouseButton @param {number} b */
  static buttonOf(b) {
    return b === 0 ? BTN_LEFT : b === 1 ? BTN_CENTER : b === 2 ? BTN_RIGHT : 0;
  }
  /** @param {number} next */
  updateMods(next) {
    if (next !== this.mods && this.mod && !this.exited) {
      this.mods = next;
      this.mod._appcui_browser_shift_state(next);
      this.wake();
    }
  }

  installInput() {
    const c = this.canvas;
    c.tabIndex = 0;
    c.style.outline = 'none';

    c.addEventListener('keydown', (e) => {
      if (!this.mod || this.exited) return;
      e.preventDefault();
      const mods = this.modsOf(e);
      if (MODIFIER_CODES.has(e.code)) { this.updateMods(mods); return; }
      this.mods = mods;
      // AltGr arrives as Ctrl+Alt on Windows: it produces text, not a Ctrl+Alt chord -> keep only Shift
      const altGr = e.getModifierState('AltGraph');
      const keyMods = altGr ? (mods & MOD_SHIFT) : mods;
      const code = KEY[e.code] ?? 0;
      let ch = 0;
      if (e.key.length === 1 && (keyMods & (MOD_CTRL | MOD_ALT)) === 0) ch = e.key.charCodeAt(0);
      const key = code === 0 ? 0 : (code | keyMods);
      if (this.mod._appcui_browser_key(key >>> 0, ch >>> 0)) this.wake();
    });
    c.addEventListener('keyup', (e) => { if (MODIFIER_CODES.has(e.code)) this.updateMods(this.modsOf(e)); });
    c.addEventListener('blur', () => this.updateMods(0)); // never leave a stuck Ctrl / Alt / Shift

    c.addEventListener('pointerdown', (e) => {
      if (!this.mod || this.exited) return;
      c.focus();
      c.setPointerCapture(e.pointerId); // keep receiving moves while dragging outside the canvas
      this.cellOf(e);
      let btn = AppCUIHost.buttonOf(e.button);
      const now = e.timeStamp;
      if (btn === BTN_LEFT && now - this.lastDownTime < DOUBLE_CLICK_MS &&
          this.lastCellX === this.lastDownX && this.lastCellY === this.lastDownY) {
        btn |= BTN_DOUBLE;
        this.lastDownTime = 0;
      } else {
        this.lastDownTime = now; this.lastDownX = this.lastCellX; this.lastDownY = this.lastCellY;
      }
      this.lastButtons = AppCUIHost.buttonsOf(e.buttons);
      if (this.mod._appcui_browser_mouse(MOUSE_DOWN, this.lastCellX, this.lastCellY, btn, this.modsOf(e))) this.wake();
      e.preventDefault();
    });
    c.addEventListener('pointerup', (e) => {
      if (!this.mod || this.exited) return;
      this.cellOf(e);
      this.lastButtons = AppCUIHost.buttonsOf(e.buttons);
      if (this.mod._appcui_browser_mouse(MOUSE_UP, this.lastCellX, this.lastCellY,
          AppCUIHost.buttonOf(e.button), this.modsOf(e))) this.wake();
    });
    c.addEventListener('pointermove', (e) => {
      if (!this.mod || this.exited) return;
      const px = this.lastCellX, py = this.lastCellY;
      this.cellOf(e);
      const buttons = AppCUIHost.buttonsOf(e.buttons);
      if (px === this.lastCellX && py === this.lastCellY && buttons === this.lastButtons) return; // same cell
      this.lastButtons = buttons;
      if (this.mod._appcui_browser_mouse(MOUSE_MOVE, this.lastCellX, this.lastCellY, buttons, this.modsOf(e))) this.wake();
    });
    c.addEventListener('wheel', (e) => {
      if (!this.mod || this.exited) return;
      e.preventDefault();
      this.cellOf(e);
      const dir = e.deltaY > 0 ? 2 : e.deltaY < 0 ? 1 : e.deltaX > 0 ? 4 : e.deltaX < 0 ? 3 : 0;
      if (dir && this.mod._appcui_browser_wheel(this.lastCellX, this.lastCellY, dir, this.modsOf(e))) this.wake();
    }, { passive: false });
    c.addEventListener('contextmenu', (e) => e.preventDefault());
  }
}

/**
 * Boots an AppCUI application compiled with the browser frontend.
 * @param {(moduleArg: object) => Promise<unknown>} createModule   the factory exported by <app>.mjs
 * @param {HTMLCanvasElement} canvas
 * @param {HTMLElement} container
 * @param {AppCUIHostOptions} options
 * @returns {Promise<AppCUIHost>}
 */
export async function startAppCUI(createModule, canvas, container, options) {
  await document.fonts.load(options.fontSizeCss + 'px ' + options.fontFamily); // metrics must come from the real font
  const host = new AppCUIHost(canvas, container, options);
  /** @type {any} */
  const moduleArg = { appcui: host }; // read by the EM_JS imports as Module["appcui"]
  // Resolve as soon as the runtime is ready, not when the module promise settles: with JSPI that promise waits for
  // main(), which may be suspended in a modal window shown before Application::Run().
  const attached = new Promise((resolve) => {
    moduleArg.onRuntimeInitialized = () => { host.attach(moduleArg); resolve(undefined); }; // moduleArg is `Module`
  });
  const failed = createModule(moduleArg).then(() => new Promise(() => {}), (err) => { throw err; });
  await Promise.race([attached, failed]);
  canvas.focus();
  return host;
}
