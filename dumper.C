#include <cstdio>

std::vector<G4S_Particle> pvec, *pvec_ptr = &pvec;
TBranch *b = nullptr;

void dump_setup_branch()
{
  b = T->GetBranch("p");
  b->SetAddress(&pvec_ptr);
  b->GetEntry(0);
  printf("Loaded event 0, size of particle vector=%d, number of primaries=%d\n",
	 (int) pvec.size(), pvec[0].n_daughters());
}

void dump_json(std::vector<G4S_Particle> &out, const char *fname)
{
  TString json = TBufferJSON::ToJSON(&out);
  printf("Created JSON object, length=%d\n", (int) json.Length());
  FILE *fp = fopen(fname, "w");
  json.Puts(fp);
  fclose(fp);
  printf("Written JSON to '%s'\n", fname);
}

// -- primaries only

void dump_primaries()
{
  dump_setup_branch();

  std::vector<G4S_Particle> out;
  // element 0 holds range of primaries
  int n_to_copy = 1 + pvec[0].n_daughters();
  out.reserve(n_to_copy);
  for (int i = 0; i < n_to_copy; ++i)
  {
    out.push_back( pvec[i] );
  }

  dump_json(out, "primaries.json");
}

// -- primaries and secondaries

void append_secondaries_of(int pi, std::vector<G4S_Particle> &out)
{
  // Assumes primaries have been copied to same positions as in pvec.
  // Appends secondaries of primary 'pi' to the end of 'out' vector.
  // Re-numbers daugter range for the primary.
  // In this case the parent indices of secondaries are correct.
  // Note -- for recursive dump one would need an additional index map.

  int dbeg = out.size();
  auto &p = out[pi];
  for (int i = p.m_daughters_begin; i != p.m_daughters_end; ++i) {
    out.push_back( pvec[i] );
  }
  p.m_daughters_begin = dbeg;
  p.m_daughters_end = out.size();
}

void dump_primaries_and_scondaries()
{
  dump_setup_branch();

  std::vector<G4S_Particle> out;
  // element 0 holds range of primaries
  int n_to_copy = 1 + pvec[0].n_daughters();
  out.reserve(n_to_copy);
  int n_daughters = 0;
  for (int i = 0; i < n_to_copy; ++i)
  {
    out.push_back( pvec[i] );
    n_daughters += pvec[i].n_daughters();
  }
  out.reserve(n_to_copy + n_daughters);
  for (int i = 1; i < n_to_copy; ++i)
  {
    append_secondaries_of(i, out);
  }

  dump_json(out, "primaries-and-secondaries.json");
}

// -- dump all

void dump_all()
{
  dump_setup_branch();
  dump_json(pvec, "all-particles.json");
}
