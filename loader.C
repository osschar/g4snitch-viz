TFile *f = nullptr;
TTree *T = nullptr;

void loader(const char *fname="Mix500-0.5-20GeV-origin.root")
{
  gSystem->Load("libRootG4Snitch.so");
  f = TFile::Open(fname);
  T = (TTree*) f->Get("T");

  // Need two-level initialization as dictionaries need to be loaded before
  // processing of code that uses G4S types.
  gROOT->LoadMacro("dumper.C");
}
