#!/usr/bin/env python3
"""Serves the AppCUI web examples (the folder of this script) over HTTP and opens one of them in the browser.

Browsers refuse to load ES modules and WebAssembly from file:// URLs, so <Example>.html must be served over HTTP.

    python serve.py                 # opens the list of examples
    python serve.py Buttons         # opens Buttons.html
    python serve.py Buttons --port 9000 --no-browser
"""
import argparse
import functools
import http.server
import pathlib
import sys
import webbrowser

ROOT = pathlib.Path(__file__).resolve().parent

# Explicit types: on Windows, Python's mimetypes reads the registry, which often maps .mjs to text/plain - and browsers
# refuse to run a module served as text/plain.
MIME_TYPES = {
    ".html": "text/html; charset=utf-8",
    ".js": "text/javascript",
    ".mjs": "text/javascript",
    ".wasm": "application/wasm",
    ".ttf": "font/ttf",
}


class Handler(http.server.SimpleHTTPRequestHandler):
    def guess_type(self, path):
        return MIME_TYPES.get(pathlib.Path(path).suffix.lower(), "application/octet-stream")

    def end_headers(self):
        self.send_header("Cache-Control", "no-store")  # always load the latest build
        super().end_headers()

    def log_message(self, format, *args):
        pass  # quiet: only the URL printed at startup matters


def main():
    examples = sorted(p.stem for p in ROOT.glob("*.html"))
    parser = argparse.ArgumentParser(description="Serve the AppCUI web examples over HTTP.")
    parser.add_argument("example", nargs="?", help="example to open (one of: " + ", ".join(examples) + ")")
    parser.add_argument("--port", type=int, default=8765, help="port on 127.0.0.1 (default: 8765, 0 = any free port)")
    parser.add_argument("--no-browser", action="store_true", help="do not open a browser")
    args = parser.parse_args()

    if args.example and args.example not in examples:
        sys.exit(f"Unknown example '{args.example}'. Available: {', '.join(examples)}")

    handler = functools.partial(Handler, directory=str(ROOT))
    try:
        server = http.server.ThreadingHTTPServer(("127.0.0.1", args.port), handler)
    except OSError:
        server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), handler)  # port busy -> any free port

    url = f"http://127.0.0.1:{server.server_address[1]}/" + (f"{args.example}.html" if args.example else "")
    print(f"Serving {ROOT} at {url}  (Ctrl+C to stop)", flush=True)
    if not args.no_browser:
        webbrowser.open(url)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
