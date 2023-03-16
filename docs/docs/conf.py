#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# OpenSRA - General documentation build configuration file
#


import os, sys
from datetime import datetime

app_name = os.path.expandvars("$SIMDOC_APP")


app_name = 'OpenSRA'

os.environ["SIMDOC_APP"] = app_name
os.environ["SIMCENTER_DEV"] = os.path.abspath("../../")


app_abrev = app_name.split("-")[0].replace("Tool", "")

app_name2 = app_name.replace("Tool", "")


# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
sys.path.append(os.path.abspath("./sphinx_ext/"))
sys.path.append(os.path.abspath("./modules/"))
# Add files for the example page template to path
sys.path.append(os.path.abspath("./modules/tmpl_0007/"))

# -----------------------------------------------------------------------------

# Add any Sphinx extension modules 
extensions =  [
    "sphinx-jsonschema",
    "sphinxcontrib.bibtex",
    "toctree_filter",
    "sphinxcontrib.images",
    "sphinx.ext.extlinks",
    "sphinxcontrib.images",
    "rendre.sphinx",
    "sphinx.ext.autodoc",
    "crate.sphinx.csv",
    "sphinx_panels",
    #"sphinxcontrib.spelling",
    'sphinx_toolbox.collapse',
    'sphinx_tabs.tabs',
]

source_suffix = {
    ".rst": "restructuredtext",
}

numfig = True
numfig_secnum_depth = 4

math_number_all = True
math_eqref_format = "({number})"
math_numfig = True

html_theme_options = {
    "logo_only": True,
    "prev_next_buttons_location": None,
    "style_nav_header_background": "#F2F2F2",
}
html_logo = f"common/figures/{app_name2}-Logo.png"

# Some apps overwrite the copyright to include additional entities
copyright = f"{str(datetime.today().year)}, The Regents of the University of California"

tags.add(f"{app_abrev}_app")

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.

# TODO: try to consolidate this to have no more than two exclude patterns per app
exclude_patterns = (
    [
        "**/OpenSRA*",
        "common/user_manual/examples/desktop/gallery.rst",
    ]
#    + [f"**{app}*" for app in APPS if app != app_name]
#    + [f'**{app.replace("-","")}*' for app in APPS if app != app_name]
)

toc_filter_exclusions = [
    "OpenSRA",
]



# -- Project information -----------------------------------------------------

#
# Basic app-specific links and data
#---------------------------------------


rst_prolog = f"""
.. |figDownload| replace:: :numref:`figDownload-{app_abrev}`
.. |figDownloadWin| replace:: :numref:`figDownloadWin-{app_abrev}`
.. |figGenericUI| replace:: :numref:`figGenericUI-{app_abrev}`
.. |figUI| replace:: :numref:`figUI-{app_abrev}`
.. |figWinUI| replace:: :numref:`figWinUI-{app_abrev}`
.. |app| replace:: {app_name2} app
.. |appName| replace:: {app_name2} app
.. |short tool id| replace:: {app_name2}
.. |short tool name| replace:: {app_name2} app
.. |tool github link| replace:: `{app_name2} Github page`_
.. |githubLink| replace:: `{app_name2} Github page`_
.. |app requirements| replace:: :ref:`lblRequirements`
.. _{app_name2} Github page: https://github.com/NHERI-SimCenter/{app_name}
.. |user survey link| replace:: `user survey`_
.. _user survey: https://docs.google.com/forms/d/e/1FAIpQLSfh20kBxDmvmHgz9uFwhkospGLCeazZzL770A2GuYZ2KgBZBA/viewform?usp=sf_link
.. |ResearchTools| replace:: `OpenSRA`
.. _SimCenter Research Tools: https://simcenter.designsafe-ci.org/research-tools/overview/


"""

external_links = {
    "github": f"https://github.com/NHERI-SimCenter/OpenSRA",
}
# Create inline :github: directive for convenient linking to files on github
extlinks = {
    "github": (f'{external_links["github"]}/tree/master/%s', f"Github"),
}


#examples_url = f"https://github.com/NHERI-SimCenter/{app_name}/tree/master/Examples/"
#
#
#
#examples_url = f"https://github.com/NHERI-SimCenter/R2DExamples/tree/master/"
#extlinks.update(
#    {
#        f"r2dt-{i:04}": (f"{examples_url}/r2dt-{i:04}/%s", f"r2dt-{i:04}")
#        for i in range(1, 20)
#    }
#)


#
# App-specific settings
#------------------------------------------------------

project = "OpenSRA Seismic Risk Assessment Tool"

author = "Barry Zheng, Stevan Gavrilovic"
sync_examples = False


toc_filter_exclusions.remove("OpenSRA")
toc_filter_exclude = toc_filter_exclusions

exclude_patterns.remove("**/OpenSRA*")

rst_prolog += f"""\
.. |full tool name| replace:: OpenSRA Seismic Risk Assessment Tool
.. |tool version| replace:: 1.0,
.. |contact person| replace:: Barry Zheng, Slate Geotechnical, bzheng@slate.com

"""
#example_config.update(
#    {
#        "include-item": [
#            "r2dt-0001",
#            "r2dt-0002",
#            "r2dt-0003",
#            "r2dt-0004",
#            "r2dt-0005",
#            "r2dt-0006",
#            "r2dt-0007"
#        ]
#    }
#)
html_theme_options.update(
    {
        "analytics_id": "...",  # TODO: add analytics ID
    }
)




# -- General configuration ----------------------------------------------

rst_prolog += f"""
.. |developers| replace:: {", ".join(f"**{auth}** " for auth in author.split(", "))}
"""


bibtex_bibfiles = ["common/references.bib"]

# Add any paths that contain templates here, relative to this directory.
templates_path = ["_templates"]

exclude_patterns += ["_build/*", "Thumbs.db", ".DS_Store", "**/_archive/*"]

# -- Options for HTML output ---------------------------------------------

html_theme = "sphinx_rtd_theme"

html_css_files = ["css/custom.css"]

html_secnum_suffix = " "

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ["_static", "_static/css/"]

# -- Options for LATEX output -------------------------------------------


latex_elements = {"extraclassoptions": "openany,oneside"}
latex_documents = [
    (
        "index",
        app_name + ".tex",                # tex output file
        project,                          # Document title
        author.replace(", ", " \\and "),  # authors
        "manual",                         # latex theme
    )
]
latex_logo = "common/figures/OpenSRA-Logo.png"


spelling_word_list_filename = ["spelling.txt"]

## sync files for examples
#if sync_examples:
#    # TODO: Temporary fix
#    if not example_config:
#        import yaml
#        with open("../examples.yaml","r") as f:
#            example_config = yaml.load(f,Loader=yaml.Loader)["HydroUQ"]
#    # Load the `sync_files` routine from ./modules/sync_files.py
#    from sync_files import sync_files
#    sync_files(
#        src_dir=os.path.abspath(f"../../{app_name}/Examples"),
#        dst_dir="common/user_manual/examples/desktop",
#        config=example_config,
#    )

