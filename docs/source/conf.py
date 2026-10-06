# Sphinx configuration of the AppCUI documentation.
#
# Build (from the docs folder):   pip install -r requirements.txt   then   make html   (or: make.bat html)
# The API reference is generated from AppCUI/include/AppCUI.hpp by Doxygen + Breathe (see _ext/appcui_api.py);
# set APPCUI_DOCS_SKIP_API=1 to build only the guides when Doxygen is not installed.

import re
import sys
from pathlib import Path

DOCS_DIR = Path(__file__).resolve().parent.parent  # .../AppCUI/docs
REPO_DIR = DOCS_DIR.parent  # .../AppCUI
sys.path.insert(0, str(Path(__file__).resolve().parent / "_ext"))

# -- Project information -----------------------------------------------------

project = "AppCUI"
author = "Gavriluț Dragoș"
copyright = "2021-2026, Gavriluț Dragoș and the AppCUI contributors"

# the version is the one of the header (APPCUI_VERSION), so the documentation always names the code it describes
_header = (REPO_DIR / "AppCUI" / "include" / "AppCUI.hpp").read_text(encoding="utf-8")
_match = re.search(r'#define APPCUI_VERSION "([0-9]+\.[0-9]+\.[0-9]+)"', _header)
release = _match.group(1) if _match else "unknown"
version = ".".join(release.split(".")[:2])

# -- General configuration ---------------------------------------------------

extensions = [
    "breathe",
    "appcui_api",
]

templates_path = []
exclude_patterns = []
primary_domain = "cpp"
highlight_language = "c++"
nitpicky = False

# -- API reference (Doxygen XML -> Breathe) ------------------------------------

appcui_doxyfile = str(DOCS_DIR / "Doxyfile")
appcui_header = str(REPO_DIR / "AppCUI" / "include" / "AppCUI.hpp")
appcui_api_output = "api"
appcui_api_namespaces = [
    "AppCUI::Application",
    "AppCUI::Controls",
    "AppCUI::Controls::Factory",
    "AppCUI::Controls::Handlers",
    "AppCUI::Graphics",
    "AppCUI::Graphics::ProgressStatus",
    "AppCUI::Input",
    "AppCUI::Utils",
    "AppCUI::Utils::Number",
    "AppCUI::OS",
    "AppCUI::Dialogs",
    "AppCUI::Log",
    "AppCUI::Endian",
    "AppCUI",
]

breathe_projects = {"AppCUI": str(DOCS_DIR / "build" / "doxygen" / "xml")}
breathe_default_project = "AppCUI"
breathe_default_members = ()
breathe_show_include = False
breathe_show_define_initializer = False
breathe_show_enumvalue_initializer = True

# -- HTML output -------------------------------------------------------------

html_theme = "sphinx_rtd_theme"
html_title = f"AppCUI {release}"
html_logo = "../logo.png"
html_static_path = ["_static"]
html_css_files = ["custom.css"]
html_theme_options = {
    "navigation_depth": 3,
    "collapse_navigation": True,
}
