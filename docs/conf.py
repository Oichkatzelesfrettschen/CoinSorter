import os, sys
project = 'CoinSorter'
extensions = ['breathe']
html_theme = 'alabaster'
source_suffix = '.rst'
master_doc = 'index'

# Breathe integration
breathe_projects = { 'CoinSorter': os.path.abspath('../build/docs/xml') }
breathe_default_project = 'CoinSorter'
