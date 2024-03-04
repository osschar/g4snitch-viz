### Using plain ROOT commands

```
# setup ROOT environment
. [path-to-root]/bin/thisroot.sh
# alternatively, on lxplus or where cvmfs is available, source cvmfs-root-init.env
. cvmfs-root-init.env

# build dictionaries
make dataformats

# get testing data in ROOT format
make get-data-root
# get testing data in JSON format
make get-data-json

# start root
root.exe
  # load the library and dictionary
  gSystem->Load("libRootG4Snitch.so")
  # open data file
  TFile::Open("Mix50-5-20GeV.root")
  # print selected entries from the tree / particle vector
  T->Scan("m_pdg:m_p_beg.fT:m_x_beg.fT:m_x_end.fT:m_parent:n_daughters():m_daughters_begin:m_daughters_end:m_g4_id:m_g4_level:m_was_tracked:m_p_beg.fT - m_mass")
```

### Using loader.C script and second level dumper.C

```
# Init env / build dictionaries as above

root.exe loader.C
  # loader.C loads the data-format & dictionary library, loads the file, then loads dumper.C
  # one can now call functions from dumper.C:
  dump_primaries()

--> this is the output + json file:
Loaded event 0, size of particle vector=20531, number of primaries=42
Created JSON object, length=46756
Written JSON to 'primaries.json'
```

`loader.C` can take a ROOT file name as argument ... but it needs to be quoted to escape the parenthesis and the file-name string quotation marks like this:
```
root.exe loader.C'("Mix50-5-20GeV.root")'
```
