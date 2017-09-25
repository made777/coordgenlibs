/*
 Contributors: Nicola Zonta
 Copyright Schrodinger, LLC. All rights reserved
 */

#include "sketcherMinimizerBond.h"
#include "sketcherMinimizerAtom.h"
#include "sketcherMinimizerMaths.h"
#include "sketcherMinimizerMolecule.h"
#include "sketcherMinimizerRing.h"

using namespace std;

sketcherMinimizerAtom* sketcherMinimizerBond::startAtomCIPFirstNeighbor() const
{
    if (bondOrder != 2)
        return NULL;
    sketcherMinimizerAtom* a = startAtom;
    if (a->neighbors.size() == 2) {
        if (a->neighbors[0] == endAtom)
            return a->neighbors[1];
        else
            return a->neighbors[0];
    } else if (a->neighbors.size() == 3) {
        std::vector<sketcherMinimizerAtom*> ats;
        foreach (sketcherMinimizerAtom* n, a->neighbors) {
            if (n != endAtom)
                ats.push_back(n);
        }
        if (ats.size() == 2)
            return sketcherMinimizerAtom::CIPPriority(ats[0], ats[1],
                                                      startAtom);
        return NULL;
    } else
        return NULL;
}

sketcherMinimizerAtom* sketcherMinimizerBond::endAtomCIPFirstNeighbor() const
{
    if (bondOrder != 2)
        return NULL;
    sketcherMinimizerAtom* a = endAtom;
    if (a->neighbors.size() == 2) {
        if (a->neighbors[0] == startAtom)
            return a->neighbors[1];
        else
            return a->neighbors[0];
    } else if (a->neighbors.size() == 3) {
        std::vector<sketcherMinimizerAtom*> ats;
        foreach (sketcherMinimizerAtom* n, a->neighbors) {
            if (n != startAtom)
                ats.push_back(n);
        }
        if (ats.size() == 2)
            return sketcherMinimizerAtom::CIPPriority(ats[0], ats[1], endAtom);
        return NULL;
    } else
        return NULL;
}

bool sketcherMinimizerBond::checkStereoChemistry() const
{
    if (!isStereo())
        return true;
    if (isInSmallRing())
        return true;
    sketcherMinimizerAtom* firstCIPNeighborStart = startAtomCIPFirstNeighbor();
    if (firstCIPNeighborStart == NULL)
        return true;
    sketcherMinimizerAtom* firstCIPNeighborEnd = endAtomCIPFirstNeighbor();
    if (firstCIPNeighborEnd == NULL)
        return true;
    return (sketcherMinimizerMaths::sameSide(
                firstCIPNeighborStart->getCoordinates(),
                firstCIPNeighborEnd->getCoordinates(),
                getStartAtom()->getCoordinates(),
                getEndAtom()->getCoordinates()) == isZ);
}

bool sketcherMinimizerBond::isInSmallRing() const
{
    for (auto ring : rings) {
        if (!ring->isMacrocycle())
            return true;
    }
    return false;
}

bool sketcherMinimizerBond::isInMacrocycle() const
{
    for (auto ring : rings) {
        if (ring->isMacrocycle())
            return true;
    }
    return false;
}

bool sketcherMinimizerBond::isTerminal() const
{
    return (getStartAtom()->getBonds().size() == 1 ||
            getEndAtom()->getBonds().size() == 1);
}

bool sketcherMinimizerBond::isInterFragment() const
{
    if (getStartAtom()->getBonds().size() == 1)
        return false;
    if (getEndAtom()->getBonds().size() == 1)
        return false;
    if (sketcherMinimizerAtom::shareARing(getStartAtom(), getEndAtom()))
        return false;
    if (isStereo())
        return false;
    return true;
}

bool sketcherMinimizerBond::markedAsCis(sketcherMinimizerAtom* atom1,
                                        sketcherMinimizerAtom* atom2) const
{
    sketcherMinimizerAtom* firstCIPNeighbor1 = startAtomCIPFirstNeighbor();
    sketcherMinimizerAtom* firstCIPNeighbor2 = endAtomCIPFirstNeighbor();
    bool cis = isZ;
    if (atom1 != firstCIPNeighbor1 && atom1 != firstCIPNeighbor2) {
        cis = !cis;
    }
    if (atom2 != firstCIPNeighbor1 && atom2 != firstCIPNeighbor2) {
        cis = !cis;
    }
    return cis;
}

bool sketcherMinimizerBond::isStereo() const
{
    if (bondOrder != 2) {
        return false;
    }
    if (m_ignoreZE) {
        return false;
    }
    sketcherMinimizerRing* ring =
        sketcherMinimizerAtom::shareARing(getStartAtom(), getEndAtom());
    if (ring && !ring->isMacrocycle())
        return false;
    return true;
}

void sketcherMinimizerBond::flip()
{
    int totalAtomsNumber = getStartAtom()->getMolecule()->getAtoms().size();
    vector<sketcherMinimizerAtom*> atoms =
        getStartAtom()->getSubmolecule(getEndAtom());
    if (atoms.size() > totalAtomsNumber * 0.5) {
        atoms = getEndAtom()->getSubmolecule(getStartAtom());
    }
    vector<sketcherMinimizerBond*> allBonds =
        getStartAtom()->getMolecule()->getBonds();

    foreach (sketcherMinimizerAtom* atom, atoms) {
        sketcherMinimizerAtom::mirrorCoordinates(atom, this);
    }
    foreach (sketcherMinimizerBond* bond, allBonds) {
        if (find(atoms.begin(), atoms.end(), bond->getStartAtom()) !=
                atoms.end() &&
            find(atoms.begin(), atoms.end(), bond->getEndAtom()) !=
                atoms.end()) {
            bond->isWedge = !bond->isWedge;
        }
    }
}