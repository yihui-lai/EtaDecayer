// Pythia 8 example and check of EtaDecayer: pp minimum bias at 13.6 TeV in which every eta (or
// eta') is decayed by EtaDecayer into the chosen mode.
//
// Usage: etatest <mode> [nEvents=1000] [seed=1] [onePerEvent=0]
//   mode: 2mu 2egamma 2mugamma 2mu2e 4e 4mu 2e2pi 2mu2pi 2muA 2piA   (parent eta, 221)
//         eta2pi eta2pi0                                             (parent eta', 331)
//         Append ":331" to an eta mode to decay the eta' instead, e.g. 4mu:331.
//   For 2muA and 2piA a 0.25 GeV dark photon A (PDG id 9000001) decaying to e+ e- is defined.
//
// For every forced decay it checks the final state (the daughter PDG ids, in any order) and momentum conservation; for
// 2mu2e it also prints the plane-angle modulation <cos 2Phi> between the mu+ mu- and e+ e- decay planes. With
// onePerEvent = 1 it checks that every event with a parent has at least one decay into the mode (exactly one is forced;
// the others decay with Pythia's table, which for eta2pi and eta2pi0 often gives the same final state) and calls
// EtaDecayer::decayOnePerEvent() after each Pythia::next().
// Exit status: 0 if all checks pass, 1 if Pythia fails to initialize or the arguments are wrong, 2 if a check fails.
#include "Pythia8/Pythia.h"
#include "EtaDecayer/EtaDecayer.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace Pythia8;

namespace {
  constexpr int kA = 9000001;  // PDG id of the boson A in 2muA and 2piA

  // Daughter PDG ids of each mode (EtaDecayer.h, "Daughter order in the event record")
  const std::map<std::string, std::vector<int>> kDaughters = {
      {"2mu", {13, -13}},           {"2egamma", {11, -11, 22}},     {"2mugamma", {13, -13, 22}},
      {"2mu2e", {13, -13, 11, -11}}, {"4e", {11, -11, 11, -11}},     {"4mu", {13, -13, 13, -13}},
      {"2e2pi", {11, -11, 211, -211}}, {"2mu2pi", {13, -13, 211, -211}}, {"eta2pi", {221, 211, -211}},
      {"eta2pi0", {221, 111, 111}}, {"2muA", {13, -13, kA}},        {"2piA", {211, -211, kA}}};

  std::vector<int> sorted(std::vector<int> v) {
    std::sort(v.begin(), v.end());
    return v;
  }
}  // namespace

int main(int argc, char** argv) {
  std::string mode = argc > 1 ? argv[1] : "2mu2e";
  const int nEv = argc > 2 ? std::atoi(argv[2]) : 1000;
  const std::string seed = argc > 3 ? argv[3] : "1";
  const bool onePerEvent = argc > 4 && std::atoi(argv[4]) != 0;

  int parent = 221;
  if (const auto colon = mode.find(':'); colon != std::string::npos) {
    parent = std::atoi(mode.substr(colon + 1).c_str());
    mode = mode.substr(0, colon);
  }
  if (mode == "eta2pi" || mode == "eta2pi0")
    parent = 331;
  const auto known = kDaughters.find(mode);
  if (known == kDaughters.end() || (parent != 221 && parent != 331) || nEv <= 0) {
    std::fprintf(stderr, "usage: %s <mode>[:221|:331] [nEvents] [seed] [onePerEvent]\nmodes:", argv[0]);
    for (const auto& [name, ids] : kDaughters)
      std::fprintf(stderr, " %s", name.c_str());
    std::fprintf(stderr, "\n");
    return 1;
  }
  const std::vector<int> expected = sorted(known->second);

  Pythia pythia("", false);
  EtaDecayer::addSettings(pythia.settings);
  for (const std::string c : {"Beams:eCM = 13600", "SoftQCD:nonDiffractive = on", "Next:numberCount = 0",
                              "Random:setSeed = on", "EtaDecayer:filter = on"})
    pythia.readString(c);
  pythia.readString("Random:seed = " + seed);
  pythia.readString("EtaDecayer:parent = " + std::to_string(parent));
  pythia.readString("EtaDecayer:mode = " + mode);
  pythia.readString(std::string("EtaDecayer:onePerEvent = ") + (onePerEvent ? "on" : "off"));
  if (mode == "2muA" || mode == "2piA") {
    pythia.readString(std::to_string(kA) + ":all = A void 3 0 0 0.25 0. 0. 0. 0.");
    pythia.readString(std::to_string(kA) + ":addChannel = 1 1. 0 11 -11");
    pythia.readString("EtaDecayer:A_id = " + std::to_string(kA));
    pythia.readString("EtaDecayer:A_type = darkphoton");
  }
  auto decayer = std::make_shared<EtaDecayer>(&pythia, &pythia.settings);
  pythia.setDecayPtr(decayer, {parent});
  if (!pythia.init())
    return 1;

  long nForced = 0, nBad = 0, nBadEvents = 0;
  double sc2 = 0, sc2sq = 0, maxDp = 0;
  for (int iEv = 0; iEv < nEv; ++iEv) {
    if (!pythia.next())
      continue;
    if (!decayer->decayOnePerEvent()) {
      ++nBadEvents;
      continue;
    }
    const Event& ev = pythia.event;
    int nParents = 0, nForcedHere = 0;
    for (int i = 0; i < ev.size(); ++i) {
      if (ev[i].id() != parent)
        continue;
      ++nParents;
      std::vector<int> ids;
      for (int j : ev[i].daughterList())
        ids.push_back(ev[j].id());
      if (sorted(ids) != expected) {
        // With onePerEvent the other parents decay with Pythia's table
        if (!onePerEvent)
          ++nBad;
        continue;
      }
      ++nForced;
      ++nForcedHere;
      Vec4 sum;
      for (int j : ev[i].daughterList())
        sum += ev[j].p();
      maxDp = std::max(maxDp, (sum - ev[i].p()).pAbs());
      if (mode != "2mu2e")
        continue;
      // Plane angle in the parent rest frame: pair 0 = mu- mu+, pair 1 = e- e+; normal n = p(l-) x p(l+)
      Vec4 lm[2], lp[2];
      for (int j : ev[i].daughterList()) {
        const int k = ev[j].idAbs() == 13 ? 0 : 1;
        Vec4 p = ev[j].p();
        p.bstback(ev[i].p());
        (ev[j].id() > 0 ? lm[k] : lp[k]) = p;
      }
      const Vec4 n1 = cross3(lm[0], lp[0]), n2 = cross3(lm[1], lp[1]);
      const double c = dot3(n1, n2) / (n1.pAbs() * n2.pAbs()), c2 = 2 * c * c - 1;
      sc2 += c2;
      sc2sq += c2 * c2;
    }
    if (onePerEvent && nParents > 0 && nForcedHere < 1)
      ++nBadEvents;
  }

  std::printf("mode=%s parent=%d onePerEvent=%d events=%d forced_decays=%ld wrong_final_state=%ld bad_events=%ld "
              "max|dp|=%.2e GeV\n",
              mode.c_str(), parent, onePerEvent, nEv, nForced, nBad, nBadEvents, maxDp);
  if (mode == "2mu2e" && nForced > 1) {
    const double m = sc2 / nForced, e = std::sqrt((sc2sq / nForced - m * m) / nForced);
    std::printf("<cos 2Phi> = %.4f +- %.4f\n", m, e);
  }
  const bool ok = nBad == 0 && nBadEvents == 0 && nForced > 0 && maxDp < 1e-3;
  std::printf("%s\n", ok ? "PASS" : "FAIL");
  return ok ? 0 : 2;
}
