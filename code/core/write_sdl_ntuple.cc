#include "write_sdl_ntuple.h"

//________________________________________________________________________________________________________________________________
void createOutputBranches()
{
    // Setup output TTree
    ana.tx->createBranch<vector<float>>("sim_pt");
    ana.tx->createBranch<vector<float>>("sim_eta");
    ana.tx->createBranch<vector<float>>("sim_phi");
    ana.tx->createBranch<vector<float>>("sim_pca_dxy");
    ana.tx->createBranch<vector<float>>("sim_pca_dz");
    ana.tx->createBranch<vector<int>>("sim_q");
    ana.tx->createBranch<vector<int>>("sim_event");
    ana.tx->createBranch<vector<int>>("sim_pdgId");
    ana.tx->createBranch<vector<int>>("sim_bunchCrossing");
    ana.tx->createBranch<vector<int>>("sim_parentVtxIdx");

    // Sim vertex
    ana.tx->createBranch<vector<float>>("simvtx_x");
    ana.tx->createBranch<vector<float>>("simvtx_y");
    ana.tx->createBranch<vector<float>>("simvtx_z");

    ana.tx->createBranch<vector<vector<int>>>("sim_tcIdx");

    // Matched to track candidate
    ana.tx->createBranch<vector<int>>("sim_TC_matched");
    ana.tx->createBranch<vector<vector<int>>>("sim_TC_types");

    // Track candidates
    ana.tx->createBranch<vector<float>>("tc_pt");
    ana.tx->createBranch<vector<float>>("tc_eta");
    ana.tx->createBranch<vector<float>>("tc_phi");
    ana.tx->createBranch<vector<int>>("tc_isFake");
    ana.tx->createBranch<vector<int>>("tc_isDuplicate");

    if (ana.do_lower_level)
    {
        createLowerLevelOutputBranches();
    }
}

//________________________________________________________________________________________________________________________________
void createLowerLevelOutputBranches()
{
    // Matched to quadruplet
    ana.tx->createBranch<vector<int>>("sim_T4_matched");
    ana.tx->createBranch<vector<vector<int>>>("sim_T4_types");

    // T4s
    ana.tx->createBranch<vector<float>>("t4_pt");
    ana.tx->createBranch<vector<float>>("t4_eta");
    ana.tx->createBranch<vector<float>>("t4_phi");
    ana.tx->createBranch<vector<int>>("t4_isFake");
    ana.tx->createBranch<vector<int>>("t4_isDuplicate");

    // Matched to triplet
    ana.tx->createBranch<vector<int>>("sim_T3_matched");
    ana.tx->createBranch<vector<vector<int>>>("sim_T3_types");

    // T3
    ana.tx->createBranch<vector<float>>("t3_pt");
    ana.tx->createBranch<vector<float>>("t3_eta");
    ana.tx->createBranch<vector<float>>("t3_phi");
    ana.tx->createBranch<vector<int>>("t3_isFake");
    ana.tx->createBranch<vector<int>>("t3_isDuplicate");
}

//________________________________________________________________________________________________________________________________
void fillOutputBranches(SDL::Event& event)
{
    fillSimTrackOutputBranches();
    fillTrackCandidateOutputBranches(event);
    if (ana.do_lower_level)
    {
        fillLowerLevelOutputBranches(event);
    }

    ana.tx->fill();
    ana.tx->clear();

}

//________________________________________________________________________________________________________________________________
void fillSimTrackOutputBranches()
{

    // Sim tracks
    ana.tx->setBranch<vector<float>>("sim_pt", trk.sim_pt());
    ana.tx->setBranch<vector<float>>("sim_eta", trk.sim_eta());
    ana.tx->setBranch<vector<float>>("sim_phi", trk.sim_phi());
    ana.tx->setBranch<vector<float>>("sim_pca_dxy", trk.sim_pca_dxy());
    ana.tx->setBranch<vector<float>>("sim_pca_dz", trk.sim_pca_dz());
    ana.tx->setBranch<vector<int>>("sim_q", trk.sim_q());
    ana.tx->setBranch<vector<int>>("sim_event", trk.sim_event());
    ana.tx->setBranch<vector<int>>("sim_pdgId", trk.sim_pdgId());
    ana.tx->setBranch<vector<int>>("sim_bunchCrossing", trk.sim_bunchCrossing());
    ana.tx->setBranch<vector<int>>("sim_parentVtxIdx", trk.sim_parentVtxIdx());

    // simvtx
    ana.tx->setBranch<vector<float>>("simvtx_x", trk.simvtx_x());
    ana.tx->setBranch<vector<float>>("simvtx_y", trk.simvtx_y());
    ana.tx->setBranch<vector<float>>("simvtx_z", trk.simvtx_z());
}

//________________________________________________________________________________________________________________________________
void fillTrackCandidateOutputBranches(SDL::Event& event)
{

    SDL::trackCandidates& trackCandidatesInGPU = (*event.getTrackCandidates());
    SDL::tracklets& trackletsInGPU = (*event.getTracklets());
    SDL::triplets& tripletsInGPU = (*event.getTriplets());
    SDL::segments& segmentsInGPU = (*event.getSegments());
    SDL::miniDoublets& miniDoubletsInGPU = (*event.getMiniDoublets());
    SDL::hits& hitsInGPU = (*event.getHits());
    SDL::modules& modulesInGPU = (*event.getModules());

    // Did it match to track candidate?
    std::vector<int> sim_TC_matched(trk.sim_pt().size());
    std::vector<vector<int>> sim_TC_types(trk.sim_pt().size());
    std::vector<int> tc_isFake;
    std::vector<vector<int>> tc_matched_simIdx;
    std::vector<float> tc_pt;
    std::vector<float> tc_eta;
    std::vector<float> tc_phi;
    for (unsigned int idx = 0; idx <= *(modulesInGPU.nLowerModules); idx++) // "<=" because cheating to include pixel track candidate lower module
    {

        if (modulesInGPU.trackCandidateModuleIndices[idx] == -1)
        {
            continue;
        }

        unsigned int nTrackCandidates = trackCandidatesInGPU.nTrackCandidates[idx];

        if (idx == *(modulesInGPU.nLowerModules) and nTrackCandidates > 5000000)
        {
            nTrackCandidates = 5000000;
        }

        if (idx < *(modulesInGPU.nLowerModules) and nTrackCandidates > 50000)
        {
            nTrackCandidates = 50000;
        }

        for (unsigned int jdx = 0; jdx < nTrackCandidates; jdx++)
        {
            unsigned int trackCandidateIndex = modulesInGPU.trackCandidateModuleIndices[idx] + jdx; // this line causes the issue
            short trackCandidateType = trackCandidatesInGPU.trackCandidateType[trackCandidateIndex];
            unsigned int innerTrackletIdx = trackCandidatesInGPU.objectIndices[2 * trackCandidateIndex];
            unsigned int outerTrackletIdx = trackCandidatesInGPU.objectIndices[2 * trackCandidateIndex + 1];

            unsigned int innerTrackletInnerSegmentIndex = -1;
            unsigned int innerTrackletOuterSegmentIndex = -1;
            unsigned int outerTrackletOuterSegmentIndex = -1;

            float betaIn_in = 0;
            float betaOut_in = 0;
            float betaIn_out = 0;
            float betaOut_out = 0;

            if (trackCandidateType == 0) // T4T4
            {
                innerTrackletInnerSegmentIndex = trackletsInGPU.segmentIndices[2 * innerTrackletIdx];
                innerTrackletOuterSegmentIndex = trackletsInGPU.segmentIndices[2 * innerTrackletIdx + 1];
                outerTrackletOuterSegmentIndex = trackletsInGPU.segmentIndices[2 * outerTrackletIdx + 1];
                betaIn_in = trackletsInGPU.betaIn[innerTrackletIdx];
                betaOut_in = trackletsInGPU.betaOut[innerTrackletIdx];
                betaIn_out = trackletsInGPU.betaIn[outerTrackletIdx];
                betaOut_out = trackletsInGPU.betaOut[outerTrackletIdx];
            }
            else if (trackCandidateType == 1) // T4T3
            {
                innerTrackletInnerSegmentIndex = trackletsInGPU.segmentIndices[2 * innerTrackletIdx];
                innerTrackletOuterSegmentIndex = trackletsInGPU.segmentIndices[2 * innerTrackletIdx + 1];
                outerTrackletOuterSegmentIndex = tripletsInGPU.segmentIndices[2 * outerTrackletIdx + 1];
                betaIn_in = trackletsInGPU.betaIn[innerTrackletIdx];
                betaOut_in = trackletsInGPU.betaOut[innerTrackletIdx];
                betaIn_out = tripletsInGPU.betaIn[outerTrackletIdx];
                betaOut_out = tripletsInGPU.betaOut[outerTrackletIdx];
            }
            else if (trackCandidateType == 2) // T3T4
            {
                innerTrackletInnerSegmentIndex = tripletsInGPU.segmentIndices[2 * innerTrackletIdx];
                innerTrackletOuterSegmentIndex = tripletsInGPU.segmentIndices[2 * innerTrackletIdx + 1];
                outerTrackletOuterSegmentIndex = trackletsInGPU.segmentIndices[2 * outerTrackletIdx + 1];
                betaIn_in = tripletsInGPU.betaIn[innerTrackletIdx];
                betaOut_in = tripletsInGPU.betaOut[innerTrackletIdx];
                betaIn_out = trackletsInGPU.betaIn[outerTrackletIdx];
                betaOut_out = trackletsInGPU.betaOut[outerTrackletIdx];
            }

            unsigned int innerTrackletInnerSegmentInnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * innerTrackletInnerSegmentIndex];
            unsigned int innerTrackletInnerSegmentOuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * innerTrackletInnerSegmentIndex + 1];
            unsigned int innerTrackletOuterSegmentInnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * innerTrackletOuterSegmentIndex];
            unsigned int innerTrackletOuterSegmentOuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * innerTrackletOuterSegmentIndex + 1];
            unsigned int outerTrackletOuterSegmentInnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * outerTrackletOuterSegmentIndex];
            unsigned int outerTrackletOuterSegmentOuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * outerTrackletOuterSegmentIndex + 1];
            unsigned int innerTrackletInnerSegmentInnerMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * innerTrackletInnerSegmentInnerMiniDoubletIndex];
            unsigned int innerTrackletInnerSegmentInnerMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * innerTrackletInnerSegmentInnerMiniDoubletIndex + 1];
            unsigned int innerTrackletInnerSegmentOuterMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * innerTrackletInnerSegmentOuterMiniDoubletIndex];
            unsigned int innerTrackletInnerSegmentOuterMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * innerTrackletInnerSegmentOuterMiniDoubletIndex + 1];
            unsigned int innerTrackletOuterSegmentInnerMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * innerTrackletOuterSegmentInnerMiniDoubletIndex];
            unsigned int innerTrackletOuterSegmentInnerMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * innerTrackletOuterSegmentInnerMiniDoubletIndex + 1];
            unsigned int innerTrackletOuterSegmentOuterMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * innerTrackletOuterSegmentOuterMiniDoubletIndex];
            unsigned int innerTrackletOuterSegmentOuterMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * innerTrackletOuterSegmentOuterMiniDoubletIndex + 1];
            unsigned int outerTrackletOuterSegmentInnerMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * outerTrackletOuterSegmentInnerMiniDoubletIndex];
            unsigned int outerTrackletOuterSegmentInnerMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * outerTrackletOuterSegmentInnerMiniDoubletIndex + 1];
            unsigned int outerTrackletOuterSegmentOuterMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * outerTrackletOuterSegmentOuterMiniDoubletIndex];
            unsigned int outerTrackletOuterSegmentOuterMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * outerTrackletOuterSegmentOuterMiniDoubletIndex + 1];

            std::vector<int> hit_idxs = {
                (int) hitsInGPU.idxs[innerTrackletInnerSegmentInnerMiniDoubletLowerHitIndex],
                (int) hitsInGPU.idxs[innerTrackletInnerSegmentInnerMiniDoubletUpperHitIndex],
                (int) hitsInGPU.idxs[innerTrackletInnerSegmentOuterMiniDoubletLowerHitIndex],
                (int) hitsInGPU.idxs[innerTrackletInnerSegmentOuterMiniDoubletUpperHitIndex],
                (int) hitsInGPU.idxs[innerTrackletOuterSegmentInnerMiniDoubletLowerHitIndex],
                (int) hitsInGPU.idxs[innerTrackletOuterSegmentInnerMiniDoubletUpperHitIndex],
                (int) hitsInGPU.idxs[innerTrackletOuterSegmentOuterMiniDoubletLowerHitIndex],
                (int) hitsInGPU.idxs[innerTrackletOuterSegmentOuterMiniDoubletUpperHitIndex],
                (int) hitsInGPU.idxs[outerTrackletOuterSegmentInnerMiniDoubletLowerHitIndex],
                (int) hitsInGPU.idxs[outerTrackletOuterSegmentInnerMiniDoubletUpperHitIndex],
                (int) hitsInGPU.idxs[outerTrackletOuterSegmentOuterMiniDoubletLowerHitIndex],
                (int) hitsInGPU.idxs[outerTrackletOuterSegmentOuterMiniDoubletUpperHitIndex],
            };

            unsigned int iiia_idx = segmentsInGPU.innerMiniDoubletAnchorHitIndices[innerTrackletInnerSegmentIndex];
            unsigned int iooa_idx = segmentsInGPU.outerMiniDoubletAnchorHitIndices[innerTrackletOuterSegmentIndex];
            unsigned int oiia_idx = segmentsInGPU.innerMiniDoubletAnchorHitIndices[innerTrackletOuterSegmentIndex];
            unsigned int oooa_idx = segmentsInGPU.outerMiniDoubletAnchorHitIndices[outerTrackletOuterSegmentIndex];

            const float dr_in = sqrt(pow(hitsInGPU.xs[iiia_idx] - hitsInGPU.xs[iooa_idx], 2) + pow(hitsInGPU.ys[iiia_idx] - hitsInGPU.ys[iooa_idx], 2));
            const float dr_out = sqrt(pow(hitsInGPU.xs[oiia_idx] - hitsInGPU.xs[oooa_idx], 2) + pow(hitsInGPU.ys[oiia_idx] - hitsInGPU.ys[oooa_idx], 2));
            const float kRinv1GeVf = (2.99792458e-3 * 3.8);
            const float k2Rinv1GeVf = kRinv1GeVf / 2.;
            const float ptAv_in = dr_in * k2Rinv1GeVf / sin((betaIn_in + betaOut_in) / 2.);
            const float ptAv_out = dr_out * k2Rinv1GeVf / sin((betaIn_out + betaOut_out) / 2.);
            const float ptAv = (ptAv_in + ptAv_out) / 2.;

            std::vector<int> hit_types;
            if (idx == *(modulesInGPU.nLowerModules)) // Then this means this track candidate is a pLS-based
            {
                hit_types.push_back(0);
                hit_types.push_back(0);
                hit_types.push_back(0);
                hit_types.push_back(0);
            }
            else
            {
                hit_types.push_back(4);
                hit_types.push_back(4);
                hit_types.push_back(4);
                hit_types.push_back(4);
            }

            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);

            std::vector<int> module_idxs = {
                (int) hitsInGPU.moduleIndices[innerTrackletInnerSegmentInnerMiniDoubletLowerHitIndex],
                (int) hitsInGPU.moduleIndices[innerTrackletInnerSegmentInnerMiniDoubletUpperHitIndex],
                (int) hitsInGPU.moduleIndices[innerTrackletInnerSegmentOuterMiniDoubletLowerHitIndex],
                (int) hitsInGPU.moduleIndices[innerTrackletInnerSegmentOuterMiniDoubletUpperHitIndex],
                (int) hitsInGPU.moduleIndices[innerTrackletOuterSegmentInnerMiniDoubletLowerHitIndex],
                (int) hitsInGPU.moduleIndices[innerTrackletOuterSegmentInnerMiniDoubletUpperHitIndex],
                (int) hitsInGPU.moduleIndices[innerTrackletOuterSegmentOuterMiniDoubletLowerHitIndex],
                (int) hitsInGPU.moduleIndices[innerTrackletOuterSegmentOuterMiniDoubletUpperHitIndex],
                (int) hitsInGPU.moduleIndices[outerTrackletOuterSegmentInnerMiniDoubletLowerHitIndex],
                (int) hitsInGPU.moduleIndices[outerTrackletOuterSegmentInnerMiniDoubletUpperHitIndex],
                (int) hitsInGPU.moduleIndices[outerTrackletOuterSegmentOuterMiniDoubletLowerHitIndex],
                (int) hitsInGPU.moduleIndices[outerTrackletOuterSegmentOuterMiniDoubletUpperHitIndex],
            };

            bool isPixel0 = (idx == *(modulesInGPU.nLowerModules));
            // bool isPixel1 = (idx == *(modulesInGPU.nLowerModules));
            bool isPixel2 = (idx == *(modulesInGPU.nLowerModules));
            // bool isPixel3 = (idx == *(modulesInGPU.nLowerModules));
            bool isPixel4 = false;
            // bool isPixel5 = false;
            bool isPixel6 = false;
            // bool isPixel7 = false;
            bool isPixel8 = false;
            // bool isPixel9 = false;
            bool isPixel10 = false;
            // bool isPixel11 = false;

            int layer0 = modulesInGPU.layers[module_idxs[0]];
            // int layer1 = modulesInGPU.layers[module_idxs[1]];
            int layer2 = modulesInGPU.layers[module_idxs[2]];
            // int layer3 = modulesInGPU.layers[module_idxs[3]];
            int layer4 = modulesInGPU.layers[module_idxs[4]];
            // int layer5 = modulesInGPU.layers[module_idxs[5]];
            int layer6 = modulesInGPU.layers[module_idxs[6]];
            // int layer7 = modulesInGPU.layers[module_idxs[7]];
            int layer8 = modulesInGPU.layers[module_idxs[8]];
            // int layer9 = modulesInGPU.layers[module_idxs[9]];
            int layer10 = modulesInGPU.layers[module_idxs[10]];
            // int layer11 = modulesInGPU.layers[module_idxs[11]];

            int subdet0 = modulesInGPU.subdets[module_idxs[0]];
            // int subdet1 = modulesInGPU.subdets[module_idxs[1]];
            int subdet2 = modulesInGPU.subdets[module_idxs[2]];
            // int subdet3 = modulesInGPU.subdets[module_idxs[3]];
            int subdet4 = modulesInGPU.subdets[module_idxs[4]];
            // int subdet5 = modulesInGPU.subdets[module_idxs[5]];
            int subdet6 = modulesInGPU.subdets[module_idxs[6]];
            // int subdet7 = modulesInGPU.subdets[module_idxs[7]];
            int subdet8 = modulesInGPU.subdets[module_idxs[8]];
            // int subdet9 = modulesInGPU.subdets[module_idxs[9]];
            int subdet10 = modulesInGPU.subdets[module_idxs[10]];
            // int subdet11 = modulesInGPU.subdets[module_idxs[11]];

            int logicallayer0 = isPixel0 ? 0 : layer0 + 6 * (subdet0 == 4);
            // int logicallayer1 = isPixel1 ? 0 : layer1 + 6 * (subdet1 == 4);
            int logicallayer2 = isPixel2 ? 0 : layer2 + 6 * (subdet2 == 4);
            // int logicallayer3 = isPixel3 ? 0 : layer3 + 6 * (subdet3 == 4);
            int logicallayer4 = isPixel4 ? 0 : layer4 + 6 * (subdet4 == 4);
            // int logicallayer5 = isPixel5 ? 0 : layer5 + 6 * (subdet5 == 4);
            int logicallayer6 = isPixel6 ? 0 : layer6 + 6 * (subdet6 == 4);
            // int logicallayer7 = isPixel7 ? 0 : layer7 + 6 * (subdet7 == 4);
            int logicallayer8 = isPixel8 ? 0 : layer8 + 6 * (subdet8 == 4);
            // int logicallayer9 = isPixel9 ? 0 : layer9 + 6 * (subdet9 == 4);
            int logicallayer10 = isPixel10 ? 0 : layer10 + 6 * (subdet10 == 4);
            // int logicallayer11 = isPixel11 ? 0 : layer11 + 6 * (subdet11 == 4);

            int layer_binary = 0;
            layer_binary |= (1 << logicallayer0);
            layer_binary |= (1 << logicallayer2);
            layer_binary |= (1 << logicallayer4);
            layer_binary |= (1 << logicallayer6);
            layer_binary |= (1 << logicallayer8);
            layer_binary |= (1 << logicallayer10);

            // sim track matched index
            std::vector<int> matched_sim_trk_idxs = matchedSimTrkIdxs(hit_idxs, hit_types);

            for (auto &isimtrk : matched_sim_trk_idxs)
            {
                sim_TC_matched[isimtrk]++;
            }

            for (auto &isimtrk : matched_sim_trk_idxs)
            {
                sim_TC_types[isimtrk].push_back(layer_binary);
            }

            // Compute pt, eta, phi of TC
            const float pt = ptAv;
            float eta = -999;
            float phi = -999;
            if (hit_types[0] == 4)
            {
                SDL::CPU::Hit hitA(trk.ph2_x()[hit_idxs[0]], trk.ph2_y()[hit_idxs[0]], trk.ph2_z()[hit_idxs[0]]);
                SDL::CPU::Hit hitB(trk.ph2_x()[hit_idxs[11]], trk.ph2_y()[hit_idxs[11]], trk.ph2_z()[hit_idxs[11]]);
                eta = hitB.eta();
                phi = hitA.phi();
            }
            else
            {
                SDL::CPU::Hit hitA(trk.pix_x()[hit_idxs[0]], trk.pix_y()[hit_idxs[0]], trk.pix_z()[hit_idxs[0]]);
                SDL::CPU::Hit hitB(trk.ph2_x()[hit_idxs[11]], trk.ph2_y()[hit_idxs[11]], trk.ph2_z()[hit_idxs[11]]);
                eta = hitB.eta();
                phi = hitA.phi();
            }

            tc_isFake.push_back(matched_sim_trk_idxs.size() == 0);
            tc_pt.push_back(pt);
            tc_eta.push_back(eta);
            tc_phi.push_back(phi);
            tc_matched_simIdx.push_back(matched_sim_trk_idxs);

        }

    }

    ana.tx->setBranch<vector<int>>("sim_TC_matched", sim_TC_matched);
    ana.tx->setBranch<vector<vector<int>>>("sim_TC_types", sim_TC_types);

    vector<int> tc_isDuplicate(tc_matched_simIdx.size());

    for (unsigned int i = 0; i < tc_matched_simIdx.size(); ++i)
    {
        bool isDuplicate = false;
        for (unsigned int isim = 0; isim < tc_matched_simIdx[i].size(); ++isim)
        {
            if (sim_TC_matched[tc_matched_simIdx[i][isim]] > 1)
            {
                isDuplicate = true;
            }
        }
        tc_isDuplicate[i] = isDuplicate;
    }

    ana.tx->setBranch<vector<float>>("tc_pt", tc_pt);
    ana.tx->setBranch<vector<float>>("tc_eta", tc_eta);
    ana.tx->setBranch<vector<float>>("tc_phi", tc_phi);
    ana.tx->setBranch<vector<int>>("tc_isFake", tc_isFake);
    ana.tx->setBranch<vector<int>>("tc_isDuplicate", tc_isDuplicate);
}

//________________________________________________________________________________________________________________________________
void fillLowerLevelOutputBranches(SDL::Event& event)
{
    fillQuadrupletOutputBranches(event);
    fillTripletOutputBranches(event);
}

//________________________________________________________________________________________________________________________________
void fillQuadrupletOutputBranches(SDL::Event& event)
{

    SDL::tracklets& trackletsInGPU = (*event.getTracklets());
    SDL::segments& segmentsInGPU = (*event.getSegments());
    SDL::miniDoublets& miniDoubletsInGPU = (*event.getMiniDoublets());
    SDL::hits& hitsInGPU = (*event.getHits());
    SDL::modules& modulesInGPU = (*event.getModules());

    // Did it match to track candidate?
    std::vector<int> sim_T4_matched(trk.sim_pt().size());
    std::vector<vector<int>> sim_T4_types(trk.sim_pt().size());
    std::vector<int> t4_isFake;
    std::vector<vector<int>> t4_matched_simIdx;
    std::vector<float> t4_pt;
    std::vector<float> t4_eta;
    std::vector<float> t4_phi;
    const int MAX_NTRACKLET_PER_MODULE = 8000;
    for (unsigned int idx = 0; idx < *(modulesInGPU.nLowerModules); idx++) // "<=" because cheating to include pixel track candidate lower module
    {

        unsigned int nTracklets = trackletsInGPU.nTracklets[idx];

        if (idx < *(modulesInGPU.nLowerModules) and nTracklets > MAX_NTRACKLET_PER_MODULE)
        {
            nTracklets = MAX_NTRACKLET_PER_MODULE;
        }

        for (unsigned int jdx = 0; jdx < nTracklets; jdx++)
        {
            unsigned int trackletIndex = MAX_NTRACKLET_PER_MODULE * idx + jdx; // this line causes the issue
            unsigned int innerSegmentIndex = -1;
            unsigned int outerSegmentIndex = -1;

            innerSegmentIndex = trackletsInGPU.segmentIndices[2 * trackletIndex];
            outerSegmentIndex = trackletsInGPU.segmentIndices[2 * trackletIndex + 1];
            float betaIn = trackletsInGPU.betaIn[trackletIndex];
            float betaOut = trackletsInGPU.betaOut[trackletIndex];

            unsigned int innerSegmentInnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * innerSegmentIndex];
            unsigned int innerSegmentOuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * innerSegmentIndex + 1];
            unsigned int outerSegmentInnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * outerSegmentIndex];
            unsigned int outerSegmentOuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * outerSegmentIndex + 1];

            unsigned int innerSegmentInnerMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * innerSegmentInnerMiniDoubletIndex];
            unsigned int innerSegmentInnerMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * innerSegmentInnerMiniDoubletIndex + 1];
            unsigned int innerSegmentOuterMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * innerSegmentOuterMiniDoubletIndex];
            unsigned int innerSegmentOuterMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * innerSegmentOuterMiniDoubletIndex + 1];
            unsigned int outerSegmentInnerMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * outerSegmentInnerMiniDoubletIndex];
            unsigned int outerSegmentInnerMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * outerSegmentInnerMiniDoubletIndex + 1];
            unsigned int outerSegmentOuterMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * outerSegmentOuterMiniDoubletIndex];
            unsigned int outerSegmentOuterMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * outerSegmentOuterMiniDoubletIndex + 1];

            std::vector<int> hit_idxs = {
                (int) hitsInGPU.idxs[innerSegmentInnerMiniDoubletLowerHitIndex],
                (int) hitsInGPU.idxs[innerSegmentInnerMiniDoubletUpperHitIndex],
                (int) hitsInGPU.idxs[innerSegmentOuterMiniDoubletLowerHitIndex],
                (int) hitsInGPU.idxs[innerSegmentOuterMiniDoubletUpperHitIndex],
                (int) hitsInGPU.idxs[outerSegmentInnerMiniDoubletLowerHitIndex],
                (int) hitsInGPU.idxs[outerSegmentInnerMiniDoubletUpperHitIndex],
                (int) hitsInGPU.idxs[outerSegmentOuterMiniDoubletLowerHitIndex],
                (int) hitsInGPU.idxs[outerSegmentOuterMiniDoubletUpperHitIndex],
            };

            unsigned int iia_idx = segmentsInGPU.innerMiniDoubletAnchorHitIndices[innerSegmentIndex];
            unsigned int ooa_idx = segmentsInGPU.outerMiniDoubletAnchorHitIndices[outerSegmentIndex];

            const float dr = sqrt(pow(hitsInGPU.xs[iia_idx] - hitsInGPU.xs[ooa_idx], 2) + pow(hitsInGPU.ys[iia_idx] - hitsInGPU.ys[ooa_idx], 2));
            const float kRinv1GeVf = (2.99792458e-3 * 3.8);
            const float k2Rinv1GeVf = kRinv1GeVf / 2.;
            const float ptAv = dr * k2Rinv1GeVf / sin((betaIn + betaOut) / 2.);

            std::vector<int> hit_types;
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);

            std::vector<int> module_idxs = {
                (int) hitsInGPU.moduleIndices[innerSegmentInnerMiniDoubletLowerHitIndex],
                (int) hitsInGPU.moduleIndices[innerSegmentInnerMiniDoubletUpperHitIndex],
                (int) hitsInGPU.moduleIndices[innerSegmentOuterMiniDoubletLowerHitIndex],
                (int) hitsInGPU.moduleIndices[innerSegmentOuterMiniDoubletUpperHitIndex],
                (int) hitsInGPU.moduleIndices[outerSegmentInnerMiniDoubletLowerHitIndex],
                (int) hitsInGPU.moduleIndices[outerSegmentInnerMiniDoubletUpperHitIndex],
                (int) hitsInGPU.moduleIndices[outerSegmentOuterMiniDoubletLowerHitIndex],
                (int) hitsInGPU.moduleIndices[outerSegmentOuterMiniDoubletUpperHitIndex],
            };

            int layer0 = modulesInGPU.layers[module_idxs[0]];
            int layer2 = modulesInGPU.layers[module_idxs[2]];
            int layer4 = modulesInGPU.layers[module_idxs[4]];
            int layer6 = modulesInGPU.layers[module_idxs[6]];

            int subdet0 = modulesInGPU.subdets[module_idxs[0]];
            int subdet2 = modulesInGPU.subdets[module_idxs[2]];
            int subdet4 = modulesInGPU.subdets[module_idxs[4]];
            int subdet6 = modulesInGPU.subdets[module_idxs[6]];

            int logicallayer0 = layer0 + 6 * (subdet0 == 4);
            int logicallayer2 = layer2 + 6 * (subdet2 == 4);
            int logicallayer4 = layer4 + 6 * (subdet4 == 4);
            int logicallayer6 = layer6 + 6 * (subdet6 == 4);

            int layer_binary = 0;
            layer_binary |= (1 << logicallayer0);
            layer_binary |= (1 << logicallayer2);
            layer_binary |= (1 << logicallayer4);
            layer_binary |= (1 << logicallayer6);

            // sim track matched index
            std::vector<int> matched_sim_trk_idxs = matchedSimTrkIdxs(hit_idxs, hit_types);

            for (auto &isimtrk : matched_sim_trk_idxs)
            {
                sim_T4_matched[isimtrk]++;
            }

            for (auto &isimtrk : matched_sim_trk_idxs)
            {
                sim_T4_types[isimtrk].push_back(layer_binary);
            }

            // Compute pt, eta, phi of T4
            const float pt = ptAv;
            float eta = -999;
            float phi = -999;
            SDL::CPU::Hit hitA(trk.ph2_x()[hit_idxs[0]], trk.ph2_y()[hit_idxs[0]], trk.ph2_z()[hit_idxs[0]]);
            SDL::CPU::Hit hitB(trk.ph2_x()[hit_idxs[7]], trk.ph2_y()[hit_idxs[7]], trk.ph2_z()[hit_idxs[7]]);
            eta = hitB.eta();
            phi = hitA.phi();

            t4_isFake.push_back(matched_sim_trk_idxs.size() == 0);
            t4_pt.push_back(pt);
            t4_eta.push_back(eta);
            t4_phi.push_back(phi);
            t4_matched_simIdx.push_back(matched_sim_trk_idxs);

        }

    }

    ana.tx->setBranch<vector<int>>("sim_T4_matched", sim_T4_matched);
    ana.tx->setBranch<vector<vector<int>>>("sim_T4_types", sim_T4_types);

    vector<int> t4_isDuplicate(t4_matched_simIdx.size());

    for (unsigned int i = 0; i < t4_matched_simIdx.size(); ++i)
    {
        bool isDuplicate = false;
        for (unsigned int isim = 0; isim < t4_matched_simIdx[i].size(); ++isim)
        {
            if (sim_T4_matched[t4_matched_simIdx[i][isim]] > 1)
            {
                isDuplicate = true;
            }
        }
        t4_isDuplicate[i] = isDuplicate;
    }

    ana.tx->setBranch<vector<float>>("t4_pt", t4_pt);
    ana.tx->setBranch<vector<float>>("t4_eta", t4_eta);
    ana.tx->setBranch<vector<float>>("t4_phi", t4_phi);
    ana.tx->setBranch<vector<int>>("t4_isFake", t4_isFake);
    ana.tx->setBranch<vector<int>>("t4_isDuplicate", t4_isDuplicate);
}

//________________________________________________________________________________________________________________________________
void fillTripletOutputBranches(SDL::Event& event)
{

    SDL::triplets& tripletsInGPU = (*event.getTriplets());
    SDL::segments& segmentsInGPU = (*event.getSegments());
    SDL::miniDoublets& miniDoubletsInGPU = (*event.getMiniDoublets());
    SDL::hits& hitsInGPU = (*event.getHits());
    SDL::modules& modulesInGPU = (*event.getModules());

    // Did it match to track candidate?
    std::vector<int> sim_T3_matched(trk.sim_pt().size());
    std::vector<vector<int>> sim_T3_types(trk.sim_pt().size());
    std::vector<int> t3_isFake;
    std::vector<vector<int>> t3_matched_simIdx;
    std::vector<float> t3_pt;
    std::vector<float> t3_eta;
    std::vector<float> t3_phi;
    const int MAX_NTRIPLET_PER_MODULE = 5000;
    for (unsigned int idx = 0; idx < *(modulesInGPU.nLowerModules); idx++) // "<=" because cheating to include pixel track candidate lower module
    {

        unsigned int nTriplets = tripletsInGPU.nTriplets[idx];

        if (idx < *(modulesInGPU.nLowerModules) and nTriplets > MAX_NTRIPLET_PER_MODULE)
        {
            nTriplets = MAX_NTRIPLET_PER_MODULE;
        }

        for (unsigned int jdx = 0; jdx < nTriplets; jdx++)
        {
            unsigned int tripletIndex = MAX_NTRIPLET_PER_MODULE * idx + jdx; // this line causes the issue
            unsigned int innerSegmentIndex = -1;
            unsigned int outerSegmentIndex = -1;

            innerSegmentIndex = tripletsInGPU.segmentIndices[2 * tripletIndex];
            outerSegmentIndex = tripletsInGPU.segmentIndices[2 * tripletIndex + 1];
            float betaIn = tripletsInGPU.betaIn[tripletIndex];
            float betaOut = tripletsInGPU.betaOut[tripletIndex];

            unsigned int innerSegmentInnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * innerSegmentIndex];
            unsigned int innerSegmentOuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * innerSegmentIndex + 1];
            unsigned int outerSegmentInnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * outerSegmentIndex];
            unsigned int outerSegmentOuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * outerSegmentIndex + 1];

            unsigned int innerSegmentInnerMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * innerSegmentInnerMiniDoubletIndex];
            unsigned int innerSegmentInnerMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * innerSegmentInnerMiniDoubletIndex + 1];
            unsigned int innerSegmentOuterMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * innerSegmentOuterMiniDoubletIndex];
            unsigned int innerSegmentOuterMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * innerSegmentOuterMiniDoubletIndex + 1];
            unsigned int outerSegmentInnerMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * outerSegmentInnerMiniDoubletIndex];
            unsigned int outerSegmentInnerMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * outerSegmentInnerMiniDoubletIndex + 1];
            unsigned int outerSegmentOuterMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * outerSegmentOuterMiniDoubletIndex];
            unsigned int outerSegmentOuterMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * outerSegmentOuterMiniDoubletIndex + 1];

            std::vector<int> hit_idxs = {
                (int) hitsInGPU.idxs[innerSegmentInnerMiniDoubletLowerHitIndex],
                (int) hitsInGPU.idxs[innerSegmentInnerMiniDoubletUpperHitIndex],
                (int) hitsInGPU.idxs[innerSegmentOuterMiniDoubletLowerHitIndex],
                (int) hitsInGPU.idxs[innerSegmentOuterMiniDoubletUpperHitIndex],
                (int) hitsInGPU.idxs[outerSegmentInnerMiniDoubletLowerHitIndex],
                (int) hitsInGPU.idxs[outerSegmentInnerMiniDoubletUpperHitIndex],
                (int) hitsInGPU.idxs[outerSegmentOuterMiniDoubletLowerHitIndex],
                (int) hitsInGPU.idxs[outerSegmentOuterMiniDoubletUpperHitIndex],
            };

            unsigned int iia_idx = segmentsInGPU.innerMiniDoubletAnchorHitIndices[innerSegmentIndex];
            unsigned int ooa_idx = segmentsInGPU.outerMiniDoubletAnchorHitIndices[outerSegmentIndex];

            const float dr = sqrt(pow(hitsInGPU.xs[iia_idx] - hitsInGPU.xs[ooa_idx], 2) + pow(hitsInGPU.ys[iia_idx] - hitsInGPU.ys[ooa_idx], 2));
            const float kRinv1GeVf = (2.99792458e-3 * 3.8);
            const float k2Rinv1GeVf = kRinv1GeVf / 2.;
            const float ptAv = dr * k2Rinv1GeVf / sin((betaIn + betaOut) / 2.);

            std::vector<int> hit_types;
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);

            std::vector<int> module_idxs = {
                (int) hitsInGPU.moduleIndices[innerSegmentInnerMiniDoubletLowerHitIndex],
                (int) hitsInGPU.moduleIndices[innerSegmentInnerMiniDoubletUpperHitIndex],
                (int) hitsInGPU.moduleIndices[innerSegmentOuterMiniDoubletLowerHitIndex],
                (int) hitsInGPU.moduleIndices[innerSegmentOuterMiniDoubletUpperHitIndex],
                (int) hitsInGPU.moduleIndices[outerSegmentInnerMiniDoubletLowerHitIndex],
                (int) hitsInGPU.moduleIndices[outerSegmentInnerMiniDoubletUpperHitIndex],
                (int) hitsInGPU.moduleIndices[outerSegmentOuterMiniDoubletLowerHitIndex],
                (int) hitsInGPU.moduleIndices[outerSegmentOuterMiniDoubletUpperHitIndex],
            };

            int layer0 = modulesInGPU.layers[module_idxs[0]];
            int layer2 = modulesInGPU.layers[module_idxs[2]];
            int layer4 = modulesInGPU.layers[module_idxs[4]];
            int layer6 = modulesInGPU.layers[module_idxs[6]];

            int subdet0 = modulesInGPU.subdets[module_idxs[0]];
            int subdet2 = modulesInGPU.subdets[module_idxs[2]];
            int subdet4 = modulesInGPU.subdets[module_idxs[4]];
            int subdet6 = modulesInGPU.subdets[module_idxs[6]];

            int logicallayer0 = layer0 + 6 * (subdet0 == 4);
            int logicallayer2 = layer2 + 6 * (subdet2 == 4);
            int logicallayer4 = layer4 + 6 * (subdet4 == 4);
            int logicallayer6 = layer6 + 6 * (subdet6 == 4);

            int layer_binary = 0;
            layer_binary |= (1 << logicallayer0);
            layer_binary |= (1 << logicallayer2);
            layer_binary |= (1 << logicallayer4);
            layer_binary |= (1 << logicallayer6);

            // sim track matched index
            std::vector<int> matched_sim_trk_idxs = matchedSimTrkIdxs(hit_idxs, hit_types);

            for (auto &isimtrk : matched_sim_trk_idxs)
            {
                sim_T3_matched[isimtrk]++;
            }

            for (auto &isimtrk : matched_sim_trk_idxs)
            {
                sim_T3_types[isimtrk].push_back(layer_binary);
            }

            // Compute pt, eta, phi of T3
            const float pt = ptAv;
            float eta = -999;
            float phi = -999;
            SDL::CPU::Hit hitA(trk.ph2_x()[hit_idxs[0]], trk.ph2_y()[hit_idxs[0]], trk.ph2_z()[hit_idxs[0]]);
            SDL::CPU::Hit hitB(trk.ph2_x()[hit_idxs[7]], trk.ph2_y()[hit_idxs[7]], trk.ph2_z()[hit_idxs[7]]);
            eta = hitB.eta();
            phi = hitA.phi();

            t3_isFake.push_back(matched_sim_trk_idxs.size() == 0);
            t3_pt.push_back(pt);
            t3_eta.push_back(eta);
            t3_phi.push_back(phi);
            t3_matched_simIdx.push_back(matched_sim_trk_idxs);

        }

    }

    ana.tx->setBranch<vector<int>>("sim_T3_matched", sim_T3_matched);
    ana.tx->setBranch<vector<vector<int>>>("sim_T3_types", sim_T3_types);

    vector<int> t3_isDuplicate(t3_matched_simIdx.size());

    for (unsigned int i = 0; i < t3_matched_simIdx.size(); ++i)
    {
        bool isDuplicate = false;
        for (unsigned int isim = 0; isim < t3_matched_simIdx[i].size(); ++isim)
        {
            if (sim_T3_matched[t3_matched_simIdx[i][isim]] > 1)
            {
                isDuplicate = true;
            }
        }
        t3_isDuplicate[i] = isDuplicate;
    }

    ana.tx->setBranch<vector<float>>("t3_pt", t3_pt);
    ana.tx->setBranch<vector<float>>("t3_eta", t3_eta);
    ana.tx->setBranch<vector<float>>("t3_phi", t3_phi);
    ana.tx->setBranch<vector<int>>("t3_isFake", t3_isFake);
    ana.tx->setBranch<vector<int>>("t3_isDuplicate", t3_isDuplicate);
}

//________________________________________________________________________________________________________________________________
void fillOutputBranches_for_CPU(SDL::CPU::Event& event)
{
    // Sim tracks
    ana.tx->setBranch<vector<float>>("sim_pt", trk.sim_pt());
    ana.tx->setBranch<vector<float>>("sim_eta", trk.sim_eta());
    ana.tx->setBranch<vector<float>>("sim_phi", trk.sim_phi());
    ana.tx->setBranch<vector<float>>("sim_pca_dxy", trk.sim_pca_dxy());
    ana.tx->setBranch<vector<float>>("sim_pca_dz", trk.sim_pca_dz());
    ana.tx->setBranch<vector<int>>("sim_q", trk.sim_q());
    ana.tx->setBranch<vector<int>>("sim_event", trk.sim_event());
    ana.tx->setBranch<vector<int>>("sim_pdgId", trk.sim_pdgId());
    ana.tx->setBranch<vector<int>>("sim_bunchCrossing", trk.sim_bunchCrossing());
    ana.tx->setBranch<vector<int>>("sim_parentVtxIdx", trk.sim_parentVtxIdx());

    // simvtx
    ana.tx->setBranch<vector<float>>("simvtx_x", trk.simvtx_x());
    ana.tx->setBranch<vector<float>>("simvtx_y", trk.simvtx_y());
    ana.tx->setBranch<vector<float>>("simvtx_z", trk.simvtx_z());

    // Did it match to track candidate?
    std::vector<int> sim_TC_matched(trk.sim_pt().size());
    std::vector<vector<int>> sim_TC_types(trk.sim_pt().size());

    // get layer ptrs
    std::vector<SDL::CPU::Layer*> layerPtrs = event.getLayerPtrs();
    layerPtrs.push_back(&(event.getPixelLayer()));

    std::vector<int> tc_isFake;
    std::vector<vector<int>> tc_matched_simIdx;
    std::vector<float> tc_pt;
    std::vector<float> tc_eta;
    std::vector<float> tc_phi;

    // Loop over layers and access track candidates
    for (auto& layerPtr : layerPtrs)
    {

        // Track Candidate ptrs
        const std::vector<SDL::CPU::TrackCandidate*>& trackCandidatePtrs = layerPtr->getTrackCandidatePtrs();


        // Loop over trackCandidate ptrs
        for (auto& trackCandidatePtr : trackCandidatePtrs)
        {

            // hit idx
            std::vector<int> hit_idx;
            hit_idx.push_back(trackCandidatePtr->innerTrackletBasePtr()->innerSegmentPtr()->innerMiniDoubletPtr()->lowerHitPtr()->idx());
            hit_idx.push_back(trackCandidatePtr->innerTrackletBasePtr()->innerSegmentPtr()->innerMiniDoubletPtr()->upperHitPtr()->idx());
            hit_idx.push_back(trackCandidatePtr->innerTrackletBasePtr()->innerSegmentPtr()->outerMiniDoubletPtr()->lowerHitPtr()->idx());
            hit_idx.push_back(trackCandidatePtr->innerTrackletBasePtr()->innerSegmentPtr()->outerMiniDoubletPtr()->upperHitPtr()->idx());
            hit_idx.push_back(trackCandidatePtr->innerTrackletBasePtr()->outerSegmentPtr()->innerMiniDoubletPtr()->lowerHitPtr()->idx());
            hit_idx.push_back(trackCandidatePtr->innerTrackletBasePtr()->outerSegmentPtr()->innerMiniDoubletPtr()->upperHitPtr()->idx());
            hit_idx.push_back(trackCandidatePtr->innerTrackletBasePtr()->outerSegmentPtr()->outerMiniDoubletPtr()->lowerHitPtr()->idx());
            hit_idx.push_back(trackCandidatePtr->innerTrackletBasePtr()->outerSegmentPtr()->outerMiniDoubletPtr()->upperHitPtr()->idx());
            hit_idx.push_back(trackCandidatePtr->outerTrackletBasePtr()->outerSegmentPtr()->innerMiniDoubletPtr()->lowerHitPtr()->idx());
            hit_idx.push_back(trackCandidatePtr->outerTrackletBasePtr()->outerSegmentPtr()->innerMiniDoubletPtr()->upperHitPtr()->idx());
            hit_idx.push_back(trackCandidatePtr->outerTrackletBasePtr()->outerSegmentPtr()->outerMiniDoubletPtr()->lowerHitPtr()->idx());
            hit_idx.push_back(trackCandidatePtr->outerTrackletBasePtr()->outerSegmentPtr()->outerMiniDoubletPtr()->upperHitPtr()->idx());

            std::vector<int> hit_types;
            if (trackCandidatePtr->innerTrackletBasePtr()->innerSegmentPtr()->innerMiniDoubletPtr()->lowerHitPtr()->getModule().detId() == 1)
            {
                hit_types.push_back(0);
                hit_types.push_back(0);
                hit_types.push_back(0);
                hit_types.push_back(0);
            }
            else
            {
                hit_types.push_back(4);
                hit_types.push_back(4);
                hit_types.push_back(4);
                hit_types.push_back(4);
            }

            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);
            hit_types.push_back(4);

            const SDL::CPU::Module& module0 = trackCandidatePtr->innerTrackletBasePtr()->innerSegmentPtr()->innerMiniDoubletPtr()->lowerHitPtr()->getModule();
            // const SDL::CPU::Module& module1 = trackCandidatePtr->innerTrackletBasePtr()->innerSegmentPtr()->innerMiniDoubletPtr()->upperHitPtr()->getModule();
            const SDL::CPU::Module& module2 = trackCandidatePtr->innerTrackletBasePtr()->innerSegmentPtr()->outerMiniDoubletPtr()->lowerHitPtr()->getModule();
            // const SDL::CPU::Module& module3 = trackCandidatePtr->innerTrackletBasePtr()->innerSegmentPtr()->outerMiniDoubletPtr()->upperHitPtr()->getModule();
            const SDL::CPU::Module& module4 = trackCandidatePtr->innerTrackletBasePtr()->outerSegmentPtr()->innerMiniDoubletPtr()->lowerHitPtr()->getModule();
            // const SDL::CPU::Module& module5 = trackCandidatePtr->innerTrackletBasePtr()->outerSegmentPtr()->innerMiniDoubletPtr()->upperHitPtr()->getModule();
            const SDL::CPU::Module& module6 = trackCandidatePtr->innerTrackletBasePtr()->outerSegmentPtr()->outerMiniDoubletPtr()->lowerHitPtr()->getModule();
            // const SDL::CPU::Module& module7 = trackCandidatePtr->innerTrackletBasePtr()->outerSegmentPtr()->outerMiniDoubletPtr()->upperHitPtr()->getModule();
            const SDL::CPU::Module& module8 = trackCandidatePtr->outerTrackletBasePtr()->outerSegmentPtr()->innerMiniDoubletPtr()->lowerHitPtr()->getModule();
            // const SDL::CPU::Module& module9 = trackCandidatePtr->outerTrackletBasePtr()->outerSegmentPtr()->innerMiniDoubletPtr()->upperHitPtr()->getModule();
            const SDL::CPU::Module& module10 = trackCandidatePtr->outerTrackletBasePtr()->outerSegmentPtr()->outerMiniDoubletPtr()->lowerHitPtr()->getModule();
            // const SDL::CPU::Module& module11 = trackCandidatePtr->outerTrackletBasePtr()->outerSegmentPtr()->outerMiniDoubletPtr()->upperHitPtr()->getModule();

            bool isPixel0 = module0.isPixelLayerModule();
            // bool isPixel1 = module1.isPixelLayerModule();
            bool isPixel2 = module2.isPixelLayerModule();
            // bool isPixel3 = module3.isPixelLayerModule();
            bool isPixel4 = module4.isPixelLayerModule();
            // bool isPixel5 = module5.isPixelLayerModule();
            bool isPixel6 = module6.isPixelLayerModule();
            // bool isPixel7 = module7.isPixelLayerModule();
            bool isPixel8 = module8.isPixelLayerModule();
            // bool isPixel9 = module9.isPixelLayerModule();
            bool isPixel10 =module10.isPixelLayerModule();
            // bool isPixel11 =module11.isPixelLayerModule();

            int layer0 = module0.layer();
            // int layer1 = module1.layer();
            int layer2 = module2.layer();
            // int layer3 = module3.layer();
            int layer4 = module4.layer();
            // int layer5 = module5.layer();
            int layer6 = module6.layer();
            // int layer7 = module7.layer();
            int layer8 = module8.layer();
            // int layer9 = module9.layer();
            int layer10 =module10.layer();
            // int layer11 =module11.layer();

            int subdet0 = module0.subdet();
            // int subdet1 = module1.subdet();
            int subdet2 = module2.subdet();
            // int subdet3 = module3.subdet();
            int subdet4 = module4.subdet();
            // int subdet5 = module5.subdet();
            int subdet6 = module6.subdet();
            // int subdet7 = module7.subdet();
            int subdet8 = module8.subdet();
            // int subdet9 = module9.subdet();
            int subdet10 =module10.subdet();
            // int subdet11 =module11.subdet();

            int logicallayer0 = isPixel0 ? 0 : layer0  + 6 * (subdet0 == 4);
            // int logicallayer1 = isPixel1 ? 0 : layer1  + 6 * (subdet1 == 4);
            int logicallayer2 = isPixel2 ? 0 : layer2  + 6 * (subdet2 == 4);
            // int logicallayer3 = isPixel3 ? 0 : layer3  + 6 * (subdet3 == 4);
            int logicallayer4 = isPixel4 ? 0 : layer4  + 6 * (subdet4 == 4);
            // int logicallayer5 = isPixel5 ? 0 : layer5  + 6 * (subdet5 == 4);
            int logicallayer6 = isPixel6 ? 0 : layer6  + 6 * (subdet6 == 4);
            // int logicallayer7 = isPixel7 ? 0 : layer7  + 6 * (subdet7 == 4);
            int logicallayer8 = isPixel8 ? 0 : layer8  + 6 * (subdet8 == 4);
            // int logicallayer9 = isPixel9 ? 0 : layer9  + 6 * (subdet9 == 4);
            int logicallayer10 =isPixel10 ? 0 : layer10 + 6 * (subdet10 == 4);
            // int logicallayer11 =isPixel11 ? 0 : layer11 + 6 * (subdet11 == 4);

            int layer_binary = 0;
            layer_binary |= (1 << logicallayer0);
            layer_binary |= (1 << logicallayer2);
            layer_binary |= (1 << logicallayer4);
            layer_binary |= (1 << logicallayer6);
            layer_binary |= (1 << logicallayer8);
            layer_binary |= (1 << logicallayer10);

            // sim track matched index
            std::vector<int> matched_sim_trk_idxs = matchedSimTrkIdxs(hit_idx, hit_types);

            for (auto& isimtrk : matched_sim_trk_idxs)
            {
                sim_TC_matched[isimtrk]++;
            }

            for (auto& isimtrk : matched_sim_trk_idxs)
            {
                sim_TC_types[isimtrk].push_back(layer_binary);
            }

            // Compute pt, eta, phi of TC
            float pt = -999;
            float eta = -999;
            float phi = -999;
            if (hit_types[0] == 4)
            {
                SDL::CPU::Hit hitA(trk.ph2_x()[hit_idx[0]], trk.ph2_y()[hit_idx[0]], trk.ph2_z()[hit_idx[0]]);
                SDL::CPU::Hit hitB(trk.ph2_x()[hit_idx[11]], trk.ph2_y()[hit_idx[11]], trk.ph2_z()[hit_idx[11]]);
                SDL::CPU::Hit hitC(0, 0, 0);
                SDL::CPU::Hit center = SDL::CPU::MathUtil::getCenterFromThreePoints(hitA, hitB, hitC);
                float radius = center.rt();
                pt = SDL::CPU::MathUtil::ptEstimateFromRadius(radius);
                eta = hitB.eta();
                phi = hitA.phi();
            }
            else
            {
                SDL::CPU::Hit hitA(trk.pix_x()[hit_idx[0]], trk.pix_y()[hit_idx[0]], trk.pix_z()[hit_idx[0]]);
                SDL::CPU::Hit hitB(trk.ph2_x()[hit_idx[11]], trk.ph2_y()[hit_idx[11]], trk.ph2_z()[hit_idx[11]]);
                SDL::CPU::Hit hitC(0, 0, 0);
                SDL::CPU::Hit center = SDL::CPU::MathUtil::getCenterFromThreePoints(hitA, hitB, hitC);
                float radius = center.rt();
                pt = SDL::CPU::MathUtil::ptEstimateFromRadius(radius);
                eta = hitB.eta();
                phi = hitA.phi();
            }

            tc_isFake.push_back(matched_sim_trk_idxs.size() == 0);
            tc_pt.push_back(pt);
            tc_eta.push_back(eta);
            tc_phi.push_back(phi);
            tc_matched_simIdx.push_back(matched_sim_trk_idxs);

        }

    }

    ana.tx->setBranch<vector<int>>("sim_TC_matched", sim_TC_matched);
    ana.tx->setBranch<vector<vector<int>>>("sim_TC_types", sim_TC_types);

    vector<int> tc_isDuplicate(tc_matched_simIdx.size());

    for (unsigned int i = 0; i < tc_matched_simIdx.size(); ++i)
    {
        bool isDuplicate = false;
        for (unsigned int isim = 0; isim < tc_matched_simIdx[i].size(); ++isim)
        {
            if (sim_TC_matched[tc_matched_simIdx[i][isim]] > 1)
            {
                isDuplicate = true;
            }
        }
        tc_isDuplicate[i] = isDuplicate;
    }

    ana.tx->setBranch<vector<float>>("tc_pt", tc_pt);
    ana.tx->setBranch<vector<float>>("tc_eta", tc_eta);
    ana.tx->setBranch<vector<float>>("tc_phi", tc_phi);
    ana.tx->setBranch<vector<int>>("tc_isFake", tc_isFake);
    ana.tx->setBranch<vector<int>>("tc_isDuplicate", tc_isDuplicate);

    ana.tx->fill();
    ana.tx->clear();

}

//________________________________________________________________________________________________________________________________
void printTimingInformation(std::vector<std::vector<float>> timing_information)
{

    if (ana.verbose == 0)
        return;

    std::cout << showpoint;
    std::cout << fixed;
    std::cout << setprecision(2);
    std::cout << right;
    std::cout << "Timing summary" << std::endl;
    std::cout << "Evt     Hits         MD       LS      T4      T4x       pT4        T3       TC       Total" << std::endl;
    std::vector<float> timing_sum_information(7);
    for (auto&& [ievt, timing] : iter::enumerate(timing_information))
    {
        float timing_total = 0.f;
        timing_total += timing[0]*1000; // Hits
        timing_total += timing[1]*1000; // MD
        timing_total += timing[2]*1000; // LS
        timing_total += timing[3]*1000; // T4
        timing_total += timing[4]*1000; // T4x
        timing_total += timing[5]*1000; // pT4
        timing_total += timing[6]*1000; // T3
        timing_total += timing[7]*1000; // TC
        std::cout << setw(6) << ievt;
        std::cout << "   "<<setw(6) << timing[0]*1000; // Hits
        std::cout << "   "<<setw(6) << timing[1]*1000; // MD
        std::cout << "   "<<setw(6) << timing[2]*1000; // LS
        std::cout << "   "<<setw(6) << timing[3]*1000; // T4
        std::cout << "   "<<setw(6) << timing[4]*1000; // T4x
        std::cout << "   "<<setw(6) << timing[5]*1000; // pT4
        std::cout << "   "<<setw(6) << timing[6]*1000; // T3
        std::cout << "   "<<setw(6) << timing[7]*1000; // TC
        std::cout << "   "<<setw(7) << timing_total; // Total time
        std::cout << std::endl;
        timing_sum_information[0] += timing[0]*1000; // Hits
        timing_sum_information[1] += timing[1]*1000; // MD
        timing_sum_information[2] += timing[2]*1000; // LS
        timing_sum_information[3] += timing[3]*1000; // T4
        timing_sum_information[4] += timing[4]*1000; // T4x
        timing_sum_information[5] += timing[5]*1000; // pT4
        timing_sum_information[6] += timing[6]*1000; // T3
        timing_sum_information[7] += timing[7]*1000; // TC
    }
    timing_sum_information[0] /= timing_information.size(); // Hits
    timing_sum_information[1] /= timing_information.size(); // MD
    timing_sum_information[2] /= timing_information.size(); // LS
    timing_sum_information[3] /= timing_information.size(); // T4
    timing_sum_information[4] /= timing_information.size(); // T4x
    timing_sum_information[5] /= timing_information.size(); // pT4
    timing_sum_information[6] /= timing_information.size(); // T3
    timing_sum_information[7] /= timing_information.size(); // TC
    float timing_total_avg = 0.f;
    timing_total_avg += timing_sum_information[0]; // Hits
    timing_total_avg += timing_sum_information[1]; // MD
    timing_total_avg += timing_sum_information[2]; // LS
    timing_total_avg += timing_sum_information[3]; // T4
    timing_total_avg += timing_sum_information[4]; // T4x
    timing_total_avg += timing_sum_information[5]; // pT4
    timing_total_avg += timing_sum_information[6]; // T3
    timing_total_avg += timing_sum_information[7]; // T3
    std::cout << setprecision(0);
    std::cout << setw(6) << "avg";
    std::cout << "   "<<setw(6) << timing_sum_information[0]; // Hits
    std::cout << "   "<<setw(6) << timing_sum_information[1]; // MD
    std::cout << "   "<<setw(6) << timing_sum_information[2]; // LS
    std::cout << "   "<<setw(6) << timing_sum_information[3]; // T4
    std::cout << "   "<<setw(6) << timing_sum_information[4]; // T4x
    std::cout << "   "<<setw(6) << timing_sum_information[5]; // pT4
    std::cout << "   "<<setw(6) << timing_sum_information[6]; // T3
    std::cout << "   "<<setw(6) << timing_sum_information[7]; // T3
    std::cout << "   "<<setw(7) << timing_total_avg; // Average total time
    std::cout << "    "<<ana.compilation_target;
    std::cout << std::endl;

    std::cout << left;

}

//________________________________________________________________________________________________________________________________
void printQuadrupletMultiplicities(SDL::Event& event)
{
    SDL::tracklets& trackletsInGPU = (*event.getTracklets());
    SDL::modules& modulesInGPU = (*event.getModules());

    int nTracklets = 0;
    //for (unsigned int idx = 0; idx <= *(SDL::modulesInGPU->nLowerModules); idx++) // "<=" because cheating to include pixel track candidate lower module
    for (unsigned int idx = 0; idx <= *(modulesInGPU.nLowerModules); idx++) // "<=" because cheating to include pixel track candidate lower module
    {
        nTracklets += trackletsInGPU.nTracklets[idx];
    }
    std::cout <<  " nTracklets: " << nTracklets <<  std::endl;
}

//________________________________________________________________________________________________________________________________
void printHitMultiplicities(SDL::Event& event)
{
    SDL::hits& hitsInGPU = (*event.getHits());
    SDL::modules& modulesInGPU = (*event.getModules());

    int nHits = 0;
    for (unsigned int idx = 0; idx <= *(modulesInGPU.nLowerModules); idx++) // "<=" because cheating to include pixel track candidate lower module
    {
        nHits += modulesInGPU.hitRanges[4 * idx + 1] - modulesInGPU.hitRanges[4 * idx] + 1;       
        nHits += modulesInGPU.hitRanges[4 * idx + 3] - modulesInGPU.hitRanges[4 * idx + 2] + 1;
    }
    std::cout <<  " nHits: " << nHits <<  std::endl;
}

//________________________________________________________________________________________________________________________________
void printMiniDoubletMultiplicities(SDL::Event& event)
{
    SDL::miniDoublets& miniDoubletsInGPU = (*event.getMiniDoublets());
    SDL::modules& modulesInGPU = (*event.getModules());

    int nMiniDoublets = 0;
    for (unsigned int idx = 0; idx <= *(modulesInGPU.nModules); idx++) // "<=" because cheating to include pixel track candidate lower module
    {
        if(modulesInGPU.isLower[idx])
        {
            nMiniDoublets += miniDoubletsInGPU.nMDs[idx];
        }
    }
    std::cout <<  " nMiniDoublets: " << nMiniDoublets <<  std::endl;
}

//________________________________________________________________________________________________________________________________
void printAllObjects(SDL::Event& event)
{
    printMDs(event);
    printLSs(event);
    printpLSs(event);
    printT3s(event);
    printT4s(event);
    printpT4s(event);
    printTCs(event);
}

//________________________________________________________________________________________________________________________________
void printAllObjects_for_CPU(SDL::CPU::Event& event)
{
    printMDs_for_CPU(event);
    printLSs_for_CPU(event);
    printpLSs_for_CPU(event);
    printT3s_for_CPU(event);
    printT4s_for_CPU(event);
    printpT4s_for_CPU(event);
    printTCs_for_CPU(event);
}

//________________________________________________________________________________________________________________________________
void printpT4s(SDL::Event& event)
{
    SDL::tracklets& trackletsInGPU = (*event.getTracklets());
    SDL::triplets& tripletsInGPU = (*event.getTriplets());
    SDL::segments& segmentsInGPU = (*event.getSegments());
    SDL::miniDoublets& miniDoubletsInGPU = (*event.getMiniDoublets());
    SDL::hits& hitsInGPU = (*event.getHits());
    SDL::modules& modulesInGPU = (*event.getModules());
    for (unsigned int itl = 0; itl < trackletsInGPU.nTracklets[*(modulesInGPU.nLowerModules)]; ++itl)
    {

        unsigned int trackletIndex = (*(modulesInGPU.nLowerModules)) * 8000/*_N_MAX_TRACK_CANDIDATES_PER_MODULE*/ + itl;
        unsigned int InnerSegmentIndex = trackletsInGPU.segmentIndices[2 * trackletIndex];
        unsigned int OuterSegmentIndex = trackletsInGPU.segmentIndices[2 * trackletIndex + 1];

        unsigned int InnerSegmentInnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * InnerSegmentIndex];
        unsigned int InnerSegmentOuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * InnerSegmentIndex + 1];
        unsigned int OuterSegmentInnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * OuterSegmentIndex];
        unsigned int OuterSegmentOuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * OuterSegmentIndex + 1];

        unsigned int InnerSegmentInnerMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * InnerSegmentInnerMiniDoubletIndex];
        unsigned int InnerSegmentInnerMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * InnerSegmentInnerMiniDoubletIndex + 1];
        unsigned int InnerSegmentOuterMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * InnerSegmentOuterMiniDoubletIndex];
        unsigned int InnerSegmentOuterMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * InnerSegmentOuterMiniDoubletIndex + 1];
        unsigned int OuterSegmentInnerMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * OuterSegmentInnerMiniDoubletIndex];
        unsigned int OuterSegmentInnerMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * OuterSegmentInnerMiniDoubletIndex + 1];
        unsigned int OuterSegmentOuterMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * OuterSegmentOuterMiniDoubletIndex];
        unsigned int OuterSegmentOuterMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * OuterSegmentOuterMiniDoubletIndex + 1];

        unsigned int hit0 = hitsInGPU.idxs[InnerSegmentInnerMiniDoubletLowerHitIndex];
        unsigned int hit1 = hitsInGPU.idxs[InnerSegmentInnerMiniDoubletUpperHitIndex];
        unsigned int hit2 = hitsInGPU.idxs[InnerSegmentOuterMiniDoubletLowerHitIndex];
        unsigned int hit3 = hitsInGPU.idxs[InnerSegmentOuterMiniDoubletUpperHitIndex];
        unsigned int hit4 = hitsInGPU.idxs[OuterSegmentInnerMiniDoubletLowerHitIndex];
        unsigned int hit5 = hitsInGPU.idxs[OuterSegmentInnerMiniDoubletUpperHitIndex];
        unsigned int hit6 = hitsInGPU.idxs[OuterSegmentOuterMiniDoubletLowerHitIndex];
        unsigned int hit7 = hitsInGPU.idxs[OuterSegmentOuterMiniDoubletUpperHitIndex];

        std::cout <<  "VALIDATION 'pT4': " << "pT4" <<  " hit0: " << hit0 <<  " hit1: " << hit1 <<  " hit2: " << hit2 <<  " hit3: " << hit3 <<  " hit4: " << hit4 <<  " hit5: " << hit5 <<  " hit6: " << hit6 <<  " hit7: " << hit7 <<  std::endl;

    }
}

//________________________________________________________________________________________________________________________________
void printpT4s_for_CPU(SDL::CPU::Event& event)
{
    // pixelLayer
    const SDL::CPU::Layer& pixelLayer = event.getPixelLayer();

    // Quadruplet ptrs
    const std::vector<SDL::CPU::Tracklet*>& trackletPtrs = pixelLayer.getTrackletPtrs();

    // Loop over tracklet ptrs
    for (auto& trackletPtr : trackletPtrs)
    {

        // hit idx
        unsigned int hit0 = trackletPtr->innerSegmentPtr()->innerMiniDoubletPtr()->lowerHitPtr()->idx();
        unsigned int hit1 = trackletPtr->innerSegmentPtr()->innerMiniDoubletPtr()->upperHitPtr()->idx();
        unsigned int hit2 = trackletPtr->innerSegmentPtr()->outerMiniDoubletPtr()->lowerHitPtr()->idx();
        unsigned int hit3 = trackletPtr->innerSegmentPtr()->outerMiniDoubletPtr()->upperHitPtr()->idx();
        unsigned int hit4 = trackletPtr->outerSegmentPtr()->innerMiniDoubletPtr()->lowerHitPtr()->idx();
        unsigned int hit5 = trackletPtr->outerSegmentPtr()->innerMiniDoubletPtr()->upperHitPtr()->idx();
        unsigned int hit6 = trackletPtr->outerSegmentPtr()->outerMiniDoubletPtr()->lowerHitPtr()->idx();
        unsigned int hit7 = trackletPtr->outerSegmentPtr()->outerMiniDoubletPtr()->upperHitPtr()->idx();

        std::cout <<  "VALIDATION 'pT4': " << "pT4" <<  " hit0: " << hit0 <<  " hit1: " << hit1 <<  " hit2: " << hit2 <<  " hit3: " << hit3 <<  " hit4: " << hit4 <<  " hit5: " << hit5 <<  " hit6: " << hit6 <<  " hit7: " << hit7 <<  std::endl;

    }
}

//________________________________________________________________________________________________________________________________
void printMDs(SDL::Event& event)
{
    SDL::miniDoublets& miniDoubletsInGPU = (*event.getMiniDoublets());
    SDL::hits& hitsInGPU = (*event.getHits());
    SDL::modules& modulesInGPU = (*event.getModules());
    for (unsigned int idx = 0; idx <= *(modulesInGPU.nLowerModules); ++idx)
    {
        for (unsigned int jdx = 0; jdx < miniDoubletsInGPU.nMDs[2*idx]; jdx++)
        {
            unsigned int mdIdx = (2*idx) * 100 + jdx;
            unsigned int LowerHitIndex = miniDoubletsInGPU.hitIndices[2 * mdIdx];
            unsigned int UpperHitIndex = miniDoubletsInGPU.hitIndices[2 * mdIdx + 1];
            unsigned int hit0 = hitsInGPU.idxs[LowerHitIndex];
            unsigned int hit1 = hitsInGPU.idxs[UpperHitIndex];
            std::cout <<  "VALIDATION 'MD': " << "MD" <<  " hit0: " << hit0 <<  " hit1: " << hit1 <<  std::endl;
        }
        for (unsigned int jdx = 0; jdx < miniDoubletsInGPU.nMDs[2*idx+1]; jdx++)
        {
            unsigned int mdIdx = (2*idx+1) * 100 + jdx;
            unsigned int LowerHitIndex = miniDoubletsInGPU.hitIndices[2 * mdIdx];
            unsigned int UpperHitIndex = miniDoubletsInGPU.hitIndices[2 * mdIdx + 1];
            unsigned int hit0 = hitsInGPU.idxs[LowerHitIndex];
            unsigned int hit1 = hitsInGPU.idxs[UpperHitIndex];
            std::cout <<  "VALIDATION 'MD': " << "MD" <<  " hit0: " << hit0 <<  " hit1: " << hit1 <<  std::endl;
        }
    }
}

//________________________________________________________________________________________________________________________________
void printMDs_for_CPU(SDL::CPU::Event& event)
{
    // get layer ptrs
    const std::vector<SDL::CPU::Layer*> layerPtrs = event.getLayerPtrs();

    // Loop over layers and access minidoublets
    for (auto& layerPtr : layerPtrs)
    {

        // MiniDoublet ptrs
        const std::vector<SDL::CPU::MiniDoublet*>& minidoubletPtrs = layerPtr->getMiniDoubletPtrs();

        // Loop over minidoublet ptrs
        for (auto& minidoubletPtr : minidoubletPtrs)
        {

            // hit idx
            unsigned int hit0 = minidoubletPtr->lowerHitPtr()->idx();
            unsigned int hit1 = minidoubletPtr->upperHitPtr()->idx();

            std::cout <<  "VALIDATION 'MD': " << "MD" <<  " hit0: " << hit0 <<  " hit1: " << hit1 <<  std::endl;

        }

    }

}

//________________________________________________________________________________________________________________________________
void printLSs(SDL::Event& event)
{
    SDL::segments& segmentsInGPU = (*event.getSegments());
    SDL::miniDoublets& miniDoubletsInGPU = (*event.getMiniDoublets());
    SDL::hits& hitsInGPU = (*event.getHits());
    SDL::modules& modulesInGPU = (*event.getModules());
    int nSegments = 0;
    for (unsigned int i = 0; i <  *(modulesInGPU.nLowerModules); ++i)
    {
        unsigned int idx = modulesInGPU.lowerModuleIndices[i];
        nSegments += segmentsInGPU.nSegments[idx];
        for (unsigned int jdx = 0; jdx < segmentsInGPU.nSegments[idx]; jdx++)
        {
            unsigned int sgIdx = idx * 600 + jdx;
            unsigned int InnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * sgIdx];
            unsigned int OuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * sgIdx + 1];
            unsigned int InnerMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * InnerMiniDoubletIndex];
            unsigned int InnerMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * InnerMiniDoubletIndex + 1];
            unsigned int OuterMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * OuterMiniDoubletIndex];
            unsigned int OuterMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * OuterMiniDoubletIndex + 1];
            unsigned int hit0 = hitsInGPU.idxs[InnerMiniDoubletLowerHitIndex];
            unsigned int hit1 = hitsInGPU.idxs[InnerMiniDoubletUpperHitIndex];
            unsigned int hit2 = hitsInGPU.idxs[OuterMiniDoubletLowerHitIndex];
            unsigned int hit3 = hitsInGPU.idxs[OuterMiniDoubletUpperHitIndex];
            std::cout <<  "VALIDATION 'LS': " << "LS" <<  " hit0: " << hit0 <<  " hit1: " << hit1 <<  " hit2: " << hit2 <<  " hit3: " << hit3 <<  std::endl;
        }
    }
    std::cout <<  "VALIDATION nSegments: " << nSegments <<  std::endl;
}

//________________________________________________________________________________________________________________________________
void printLSs_for_CPU(SDL::CPU::Event& event)
{
    // get layer ptrs
    const std::vector<SDL::CPU::Layer*> layerPtrs = event.getLayerPtrs();

    // Loop over layers and access segments
    for (auto& layerPtr : layerPtrs)
    {

        // MiniDoublet ptrs
        const std::vector<SDL::CPU::Segment*>& segmentPtrs = layerPtr->getSegmentPtrs();

        // Loop over segment ptrs
        for (auto& segmentPtr : segmentPtrs)
        {

            // hit idx
            unsigned int hit0 = segmentPtr->innerMiniDoubletPtr()->lowerHitPtr()->idx();
            unsigned int hit1 = segmentPtr->innerMiniDoubletPtr()->upperHitPtr()->idx();
            unsigned int hit2 = segmentPtr->outerMiniDoubletPtr()->lowerHitPtr()->idx();
            unsigned int hit3 = segmentPtr->outerMiniDoubletPtr()->upperHitPtr()->idx();

            std::cout <<  "VALIDATION 'LS': " << "LS" <<  " hit0: " << hit0 <<  " hit1: " << hit1 <<  " hit2: " << hit2 <<  " hit3: " << hit3 <<  std::endl;

        }

    }

}

//________________________________________________________________________________________________________________________________
void printpLSs(SDL::Event& event)
{
    SDL::segments& segmentsInGPU = (*event.getSegments());
    SDL::miniDoublets& miniDoubletsInGPU = (*event.getMiniDoublets());
    SDL::hits& hitsInGPU = (*event.getHits());
    SDL::modules& modulesInGPU = (*event.getModules());
    unsigned int i = *(modulesInGPU.nLowerModules);
    unsigned int idx = modulesInGPU.lowerModuleIndices[i];
    int npLS = segmentsInGPU.nSegments[idx];
    for (unsigned int jdx = 0; jdx < segmentsInGPU.nSegments[idx]; jdx++)
    {
        unsigned int sgIdx = idx * 600 + jdx;
        unsigned int InnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * sgIdx];
        unsigned int OuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * sgIdx + 1];
        unsigned int InnerMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * InnerMiniDoubletIndex];
        unsigned int InnerMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * InnerMiniDoubletIndex + 1];
        unsigned int OuterMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * OuterMiniDoubletIndex];
        unsigned int OuterMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * OuterMiniDoubletIndex + 1];
        unsigned int hit0 = hitsInGPU.idxs[InnerMiniDoubletLowerHitIndex];
        unsigned int hit1 = hitsInGPU.idxs[InnerMiniDoubletUpperHitIndex];
        unsigned int hit2 = hitsInGPU.idxs[OuterMiniDoubletLowerHitIndex];
        unsigned int hit3 = hitsInGPU.idxs[OuterMiniDoubletUpperHitIndex];
        std::cout <<  "VALIDATION 'pLS': " << "pLS" <<  " hit0: " << hit0 <<  " hit1: " << hit1 <<  " hit2: " << hit2 <<  " hit3: " << hit3 <<  std::endl;
    }
    std::cout <<  "VALIDATION npLS: " << npLS <<  std::endl;
}

//________________________________________________________________________________________________________________________________
void printpLSs_for_CPU(SDL::CPU::Event& event)
{
    // get layer ptr
    SDL::CPU::Layer* layerPtr = &(event.getPixelLayer());

    // MiniDoublet ptrs
    const std::vector<SDL::CPU::Segment*>& segmentPtrs = layerPtr->getSegmentPtrs();

    // Number of pLS
    int npLS = segmentPtrs.size();

    // Loop over segment ptrs
    for (auto& segmentPtr : segmentPtrs)
    {

        // hit idx
        unsigned int hit0 = segmentPtr->innerMiniDoubletPtr()->lowerHitPtr()->idx();
        unsigned int hit1 = segmentPtr->innerMiniDoubletPtr()->upperHitPtr()->idx();
        unsigned int hit2 = segmentPtr->outerMiniDoubletPtr()->lowerHitPtr()->idx();
        unsigned int hit3 = segmentPtr->outerMiniDoubletPtr()->upperHitPtr()->idx();

        std::cout <<  "VALIDATION 'pLS': " << "pLS" <<  " hit0: " << hit0 <<  " hit1: " << hit1 <<  " hit2: " << hit2 <<  " hit3: " << hit3 <<  std::endl;

    }
    std::cout <<  "VALIDATION npLS: " << npLS <<  std::endl;

}

//________________________________________________________________________________________________________________________________
void printT3s(SDL::Event& event)
{
    SDL::triplets& tripletsInGPU = (*event.getTriplets());
    SDL::segments& segmentsInGPU = (*event.getSegments());
    SDL::miniDoublets& miniDoubletsInGPU = (*event.getMiniDoublets());
    SDL::hits& hitsInGPU = (*event.getHits());
    SDL::modules& modulesInGPU = (*event.getModules());
    int nTriplets = 0;
    for (unsigned int i = 0; i <  *(modulesInGPU.nLowerModules); ++i)
    {
        // unsigned int idx = SDL::modulesInGPU->lowerModuleIndices[i];
        nTriplets += tripletsInGPU.nTriplets[i];
        unsigned int idx = i;
        for (unsigned int jdx = 0; jdx < tripletsInGPU.nTriplets[idx]; jdx++)
        {
            unsigned int tpIdx = idx * 5000 + jdx;
            unsigned int InnerSegmentIndex = tripletsInGPU.segmentIndices[2 * tpIdx];
            unsigned int OuterSegmentIndex = tripletsInGPU.segmentIndices[2 * tpIdx + 1];
            unsigned int InnerSegmentInnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * InnerSegmentIndex];
            unsigned int InnerSegmentOuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * InnerSegmentIndex + 1];
            unsigned int OuterSegmentOuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * OuterSegmentIndex + 1];

            unsigned int hit_idx0 = miniDoubletsInGPU.hitIndices[2 * InnerSegmentInnerMiniDoubletIndex];
            unsigned int hit_idx1 = miniDoubletsInGPU.hitIndices[2 * InnerSegmentInnerMiniDoubletIndex + 1];
            unsigned int hit_idx2 = miniDoubletsInGPU.hitIndices[2 * InnerSegmentOuterMiniDoubletIndex];
            unsigned int hit_idx3 = miniDoubletsInGPU.hitIndices[2 * InnerSegmentOuterMiniDoubletIndex + 1];
            unsigned int hit_idx4 = miniDoubletsInGPU.hitIndices[2 * OuterSegmentOuterMiniDoubletIndex];
            unsigned int hit_idx5 = miniDoubletsInGPU.hitIndices[2 * OuterSegmentOuterMiniDoubletIndex + 1];

            unsigned int hit0 = hitsInGPU.idxs[hit_idx0];
            unsigned int hit1 = hitsInGPU.idxs[hit_idx1];
            unsigned int hit2 = hitsInGPU.idxs[hit_idx2];
            unsigned int hit3 = hitsInGPU.idxs[hit_idx3];
            unsigned int hit4 = hitsInGPU.idxs[hit_idx4];
            unsigned int hit5 = hitsInGPU.idxs[hit_idx5];
            std::cout <<  "VALIDATION 'T3': " << "T3" <<  " hit0: " << hit0 <<  " hit1: " << hit1 <<  " hit2: " << hit2 <<  " hit3: " << hit3 <<  " hit4: " << hit4 <<  " hit5: " << hit5 <<  std::endl;
        }
    }
    std::cout <<  "VALIDATION nTriplets: " << nTriplets <<  std::endl;
}

//________________________________________________________________________________________________________________________________
void printT3s_for_CPU(SDL::CPU::Event& event)
{
    // get layer ptrs
    const std::vector<SDL::CPU::Layer*> layerPtrs = event.getLayerPtrs();

    // Loop over layers and access triplets
    for (auto& layerPtr : layerPtrs)
    {

        // MiniDoublet ptrs
        const std::vector<SDL::CPU::Triplet*>& tripletPtrs = layerPtr->getTripletPtrs();

        // Loop over triplet ptrs
        for (auto& tripletPtr : tripletPtrs)
        {

            // hit idx
            unsigned int hit0 = tripletPtr->innerSegmentPtr()->innerMiniDoubletPtr()->lowerHitPtr()->idx();
            unsigned int hit1 = tripletPtr->innerSegmentPtr()->innerMiniDoubletPtr()->upperHitPtr()->idx();
            unsigned int hit2 = tripletPtr->innerSegmentPtr()->outerMiniDoubletPtr()->lowerHitPtr()->idx();
            unsigned int hit3 = tripletPtr->innerSegmentPtr()->outerMiniDoubletPtr()->upperHitPtr()->idx();
            unsigned int hit4 = tripletPtr->outerSegmentPtr()->outerMiniDoubletPtr()->lowerHitPtr()->idx();
            unsigned int hit5 = tripletPtr->outerSegmentPtr()->outerMiniDoubletPtr()->upperHitPtr()->idx();

            std::cout <<  "VALIDATION 'T3': " << "T3" <<  " hit0: " << hit0 <<  " hit1: " << hit1 <<  " hit2: " << hit2 <<  " hit3: " << hit3 <<  " hit4: " << hit4 <<  " hit5: " << hit5 <<  std::endl;

        }

    }

}

//________________________________________________________________________________________________________________________________
void printT4s(SDL::Event& event)
{
    SDL::tracklets& trackletsInGPU = (*event.getTracklets());
    SDL::segments& segmentsInGPU = (*event.getSegments());
    SDL::miniDoublets& miniDoubletsInGPU = (*event.getMiniDoublets());
    SDL::hits& hitsInGPU = (*event.getHits());
    SDL::modules& modulesInGPU = (*event.getModules());
    int nTracklets = 0;
    for (unsigned int i = 0; i <  *(modulesInGPU.nLowerModules); ++i)
    {
        // unsigned int idx = SDL::modulesInGPU->lowerModuleIndices[i];
        // nTracklets += trackletsInGPU.nTracklets[idx];
        nTracklets += trackletsInGPU.nTracklets[i];
        unsigned int idx = i;
        for (unsigned int jdx = 0; jdx < trackletsInGPU.nTracklets[idx]; jdx++)
        {
            unsigned int tlIdx = idx * 8000 + jdx;
            unsigned int InnerSegmentIndex = trackletsInGPU.segmentIndices[2 * tlIdx];
            unsigned int OuterSegmentIndex = trackletsInGPU.segmentIndices[2 * tlIdx + 1];
            unsigned int InnerSegmentInnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * InnerSegmentIndex];
            unsigned int InnerSegmentOuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * InnerSegmentIndex + 1];
            unsigned int OuterSegmentInnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * OuterSegmentIndex];
            unsigned int OuterSegmentOuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * OuterSegmentIndex + 1];

            unsigned int hit_idx0 = miniDoubletsInGPU.hitIndices[2 * InnerSegmentInnerMiniDoubletIndex];
            unsigned int hit_idx1 = miniDoubletsInGPU.hitIndices[2 * InnerSegmentInnerMiniDoubletIndex + 1];
            unsigned int hit_idx2 = miniDoubletsInGPU.hitIndices[2 * InnerSegmentOuterMiniDoubletIndex];
            unsigned int hit_idx3 = miniDoubletsInGPU.hitIndices[2 * InnerSegmentOuterMiniDoubletIndex + 1];
            unsigned int hit_idx4 = miniDoubletsInGPU.hitIndices[2 * OuterSegmentInnerMiniDoubletIndex];
            unsigned int hit_idx5 = miniDoubletsInGPU.hitIndices[2 * OuterSegmentInnerMiniDoubletIndex + 1];
            unsigned int hit_idx6 = miniDoubletsInGPU.hitIndices[2 * OuterSegmentOuterMiniDoubletIndex];
            unsigned int hit_idx7 = miniDoubletsInGPU.hitIndices[2 * OuterSegmentOuterMiniDoubletIndex + 1];

            unsigned int hit0 = hitsInGPU.idxs[hit_idx0];
            unsigned int hit1 = hitsInGPU.idxs[hit_idx1];
            unsigned int hit2 = hitsInGPU.idxs[hit_idx2];
            unsigned int hit3 = hitsInGPU.idxs[hit_idx3];
            unsigned int hit4 = hitsInGPU.idxs[hit_idx4];
            unsigned int hit5 = hitsInGPU.idxs[hit_idx5];
            unsigned int hit6 = hitsInGPU.idxs[hit_idx6];
            unsigned int hit7 = hitsInGPU.idxs[hit_idx7];
            std::cout <<  "VALIDATION 'T4': " << "T4" <<  " hit0: " << hit0 <<  " hit1: " << hit1 <<  " hit2: " << hit2 <<  " hit3: " << hit3 <<  " hit4: " << hit4 <<  " hit5: " << hit5 <<  " hit6: " << hit6 <<  " hit7: " << hit7 <<  std::endl;
        }
    }
    std::cout <<  "VALIDATION nTracklets: " << nTracklets <<  std::endl;
}

//________________________________________________________________________________________________________________________________
void printT4s_for_CPU(SDL::CPU::Event& event)
{
    // get layer ptrs
    const std::vector<SDL::CPU::Layer*> layerPtrs = event.getLayerPtrs();

    // Loop over layers and access tracklets
    for (auto& layerPtr : layerPtrs)
    {

        // MiniDoublet ptrs
        const std::vector<SDL::CPU::Tracklet*>& trackletPtrs = layerPtr->getTrackletPtrs();

        // Loop over tracklet ptrs
        for (auto& trackletPtr : trackletPtrs)
        {

            // hit idx
            unsigned int hit0 = trackletPtr->innerSegmentPtr()->innerMiniDoubletPtr()->lowerHitPtr()->idx();
            unsigned int hit1 = trackletPtr->innerSegmentPtr()->innerMiniDoubletPtr()->upperHitPtr()->idx();
            unsigned int hit2 = trackletPtr->innerSegmentPtr()->outerMiniDoubletPtr()->lowerHitPtr()->idx();
            unsigned int hit3 = trackletPtr->innerSegmentPtr()->outerMiniDoubletPtr()->upperHitPtr()->idx();
            unsigned int hit4 = trackletPtr->outerSegmentPtr()->innerMiniDoubletPtr()->lowerHitPtr()->idx();
            unsigned int hit5 = trackletPtr->outerSegmentPtr()->innerMiniDoubletPtr()->upperHitPtr()->idx();
            unsigned int hit6 = trackletPtr->outerSegmentPtr()->outerMiniDoubletPtr()->lowerHitPtr()->idx();
            unsigned int hit7 = trackletPtr->outerSegmentPtr()->outerMiniDoubletPtr()->upperHitPtr()->idx();

            std::cout <<  "VALIDATION 'T4': " << "T4" <<  " hit0: " << hit0 <<  " hit1: " << hit1 <<  " hit2: " << hit2 <<  " hit3: " << hit3 <<  " hit4: " << hit4 <<  " hit5: " << hit5 <<  " hit6: " << hit6 <<  " hit7: " << hit7 <<  std::endl;

        }

    }

}

//________________________________________________________________________________________________________________________________
void printTCs(SDL::Event& event)
{
    SDL::trackCandidates& trackCandidatesInGPU = (*event.getTrackCandidates());
    SDL::tracklets& trackletsInGPU = (*event.getTracklets());
    SDL::triplets& tripletsInGPU = (*event.getTriplets());
    SDL::segments& segmentsInGPU = (*event.getSegments());
    SDL::miniDoublets& miniDoubletsInGPU = (*event.getMiniDoublets());
    SDL::hits& hitsInGPU = (*event.getHits());
    SDL::modules& modulesInGPU = (*event.getModules());
    int nTrackCandidates = 0;
    for (unsigned int idx = 0; idx <= *(modulesInGPU.nLowerModules); ++idx)
    {
            if(modulesInGPU.trackCandidateModuleIndices[idx] == -1)
                continue;
        for (unsigned int jdx = 0; jdx < trackCandidatesInGPU.nTrackCandidates[idx]; jdx++)
        {
            //unsigned int trackCandidateIndex = idx * 50000/*_N_MAX_TRACK_CANDIDATES_PER_MODULE*/ + jdx;
            unsigned int trackCandidateIndex = modulesInGPU.trackCandidateModuleIndices[idx] + jdx;

            short trackCandidateType = trackCandidatesInGPU.trackCandidateType[trackCandidateIndex];

            unsigned int innerTrackletIdx = trackCandidatesInGPU.objectIndices[2 * trackCandidateIndex];
            unsigned int outerTrackletIdx = trackCandidatesInGPU.objectIndices[2 * trackCandidateIndex + 1];

            unsigned int innerTrackletInnerSegmentIndex = -1;
            unsigned int innerTrackletOuterSegmentIndex = -1;
            unsigned int outerTrackletOuterSegmentIndex = -1;

            if (trackCandidateType == 0) // T4T4
            {
                innerTrackletInnerSegmentIndex = trackletsInGPU.segmentIndices[2 * innerTrackletIdx];
                innerTrackletOuterSegmentIndex = trackletsInGPU.segmentIndices[2 * innerTrackletIdx + 1];
                outerTrackletOuterSegmentIndex = trackletsInGPU.segmentIndices[2 * outerTrackletIdx + 1];
            }
            else if (trackCandidateType == 1) // T4T3
            {
                innerTrackletInnerSegmentIndex = trackletsInGPU.segmentIndices[2 * innerTrackletIdx];
                innerTrackletOuterSegmentIndex = trackletsInGPU.segmentIndices[2 * innerTrackletIdx + 1];
                outerTrackletOuterSegmentIndex = tripletsInGPU.segmentIndices[2 * outerTrackletIdx + 1];
            }
            else if (trackCandidateType == 2) // T3T4
            {
                innerTrackletInnerSegmentIndex = tripletsInGPU.segmentIndices[2 * innerTrackletIdx];
                innerTrackletOuterSegmentIndex = tripletsInGPU.segmentIndices[2 * innerTrackletIdx + 1];
                outerTrackletOuterSegmentIndex = trackletsInGPU.segmentIndices[2 * outerTrackletIdx + 1];
            }

            unsigned int innerTrackletInnerSegmentInnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * innerTrackletInnerSegmentIndex];
            unsigned int innerTrackletInnerSegmentOuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * innerTrackletInnerSegmentIndex + 1];
            unsigned int innerTrackletOuterSegmentInnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * innerTrackletOuterSegmentIndex];
            unsigned int innerTrackletOuterSegmentOuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * innerTrackletOuterSegmentIndex + 1];
            unsigned int outerTrackletOuterSegmentInnerMiniDoubletIndex = segmentsInGPU.mdIndices[2 * outerTrackletOuterSegmentIndex];
            unsigned int outerTrackletOuterSegmentOuterMiniDoubletIndex = segmentsInGPU.mdIndices[2 * outerTrackletOuterSegmentIndex + 1];

            unsigned int innerTrackletInnerSegmentInnerMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * innerTrackletInnerSegmentInnerMiniDoubletIndex];
            unsigned int innerTrackletInnerSegmentInnerMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * innerTrackletInnerSegmentInnerMiniDoubletIndex + 1];
            unsigned int innerTrackletInnerSegmentOuterMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * innerTrackletInnerSegmentOuterMiniDoubletIndex];
            unsigned int innerTrackletInnerSegmentOuterMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * innerTrackletInnerSegmentOuterMiniDoubletIndex + 1];
            unsigned int innerTrackletOuterSegmentInnerMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * innerTrackletOuterSegmentInnerMiniDoubletIndex];
            unsigned int innerTrackletOuterSegmentInnerMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * innerTrackletOuterSegmentInnerMiniDoubletIndex + 1];
            unsigned int innerTrackletOuterSegmentOuterMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * innerTrackletOuterSegmentOuterMiniDoubletIndex];
            unsigned int innerTrackletOuterSegmentOuterMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * innerTrackletOuterSegmentOuterMiniDoubletIndex + 1];
            unsigned int outerTrackletOuterSegmentInnerMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * outerTrackletOuterSegmentInnerMiniDoubletIndex];
            unsigned int outerTrackletOuterSegmentInnerMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * outerTrackletOuterSegmentInnerMiniDoubletIndex + 1];
            unsigned int outerTrackletOuterSegmentOuterMiniDoubletLowerHitIndex = miniDoubletsInGPU.hitIndices[2 * outerTrackletOuterSegmentOuterMiniDoubletIndex];
            unsigned int outerTrackletOuterSegmentOuterMiniDoubletUpperHitIndex = miniDoubletsInGPU.hitIndices[2 * outerTrackletOuterSegmentOuterMiniDoubletIndex + 1];

            unsigned int hit0 = hitsInGPU.idxs[innerTrackletInnerSegmentInnerMiniDoubletLowerHitIndex];
            unsigned int hit1 = hitsInGPU.idxs[innerTrackletInnerSegmentInnerMiniDoubletUpperHitIndex];
            unsigned int hit2 = hitsInGPU.idxs[innerTrackletInnerSegmentOuterMiniDoubletLowerHitIndex];
            unsigned int hit3 = hitsInGPU.idxs[innerTrackletInnerSegmentOuterMiniDoubletUpperHitIndex];
            unsigned int hit4 = hitsInGPU.idxs[innerTrackletOuterSegmentInnerMiniDoubletLowerHitIndex];
            unsigned int hit5 = hitsInGPU.idxs[innerTrackletOuterSegmentInnerMiniDoubletUpperHitIndex];
            unsigned int hit6 = hitsInGPU.idxs[innerTrackletOuterSegmentOuterMiniDoubletLowerHitIndex];
            unsigned int hit7 = hitsInGPU.idxs[innerTrackletOuterSegmentOuterMiniDoubletUpperHitIndex];
            unsigned int hit8 = hitsInGPU.idxs[outerTrackletOuterSegmentInnerMiniDoubletLowerHitIndex];
            unsigned int hit9 = hitsInGPU.idxs[outerTrackletOuterSegmentInnerMiniDoubletUpperHitIndex];
            unsigned int hit10 = hitsInGPU.idxs[outerTrackletOuterSegmentOuterMiniDoubletLowerHitIndex];
            unsigned int hit11 = hitsInGPU.idxs[outerTrackletOuterSegmentOuterMiniDoubletUpperHitIndex];

            std::cout <<  "VALIDATION 'TC': " << "TC" <<  " hit0: " << hit0 <<  " hit1: " << hit1 <<  " hit2: " << hit2 <<  " hit3: " << hit3 <<  " hit4: " << hit4 <<  " hit5: " << hit5 <<  " hit6: " << hit6 <<  " hit7: " << hit7 <<  " hit8: " << hit8 <<  " hit9: " << hit9 <<  " hit10: " << hit10 <<  " hit11: " << hit11 <<  std::endl;
        }
    }
}

//________________________________________________________________________________________________________________________________
void printTCs_for_CPU(SDL::CPU::Event& event)
{
    // get layer ptrs
    std::vector<SDL::CPU::Layer*> layerPtrs = event.getLayerPtrs();
    layerPtrs.push_back(&(event.getPixelLayer()));

    // Loop over layers and access track candidates
    for (auto& layerPtr : layerPtrs)
    {

        // Track Candidate ptrs
        const std::vector<SDL::CPU::TrackCandidate*>& trackCandidatePtrs = layerPtr->getTrackCandidatePtrs();


        // Loop over trackCandidate ptrs
        for (auto& trackCandidatePtr : trackCandidatePtrs)
        {
            // hit idx
            unsigned int hit0 = trackCandidatePtr->innerTrackletBasePtr()->innerSegmentPtr()->innerMiniDoubletPtr()->lowerHitPtr()->idx();
            unsigned int hit1 = trackCandidatePtr->innerTrackletBasePtr()->innerSegmentPtr()->innerMiniDoubletPtr()->upperHitPtr()->idx();
            unsigned int hit2 = trackCandidatePtr->innerTrackletBasePtr()->innerSegmentPtr()->outerMiniDoubletPtr()->lowerHitPtr()->idx();
            unsigned int hit3 = trackCandidatePtr->innerTrackletBasePtr()->innerSegmentPtr()->outerMiniDoubletPtr()->upperHitPtr()->idx();
            unsigned int hit4 = trackCandidatePtr->innerTrackletBasePtr()->outerSegmentPtr()->innerMiniDoubletPtr()->lowerHitPtr()->idx();
            unsigned int hit5 = trackCandidatePtr->innerTrackletBasePtr()->outerSegmentPtr()->innerMiniDoubletPtr()->upperHitPtr()->idx();
            unsigned int hit6 = trackCandidatePtr->innerTrackletBasePtr()->outerSegmentPtr()->outerMiniDoubletPtr()->lowerHitPtr()->idx();
            unsigned int hit7 = trackCandidatePtr->innerTrackletBasePtr()->outerSegmentPtr()->outerMiniDoubletPtr()->upperHitPtr()->idx();
            unsigned int hit8 = trackCandidatePtr->outerTrackletBasePtr()->outerSegmentPtr()->innerMiniDoubletPtr()->lowerHitPtr()->idx();
            unsigned int hit9 = trackCandidatePtr->outerTrackletBasePtr()->outerSegmentPtr()->innerMiniDoubletPtr()->upperHitPtr()->idx();
            unsigned int hit10 = trackCandidatePtr->outerTrackletBasePtr()->outerSegmentPtr()->outerMiniDoubletPtr()->lowerHitPtr()->idx();
            unsigned int hit11 = trackCandidatePtr->outerTrackletBasePtr()->outerSegmentPtr()->outerMiniDoubletPtr()->upperHitPtr()->idx();

            std::cout <<  "VALIDATION 'TC': " << "TC" <<  " hit0: " << hit0 <<  " hit1: " << hit1 <<  " hit2: " << hit2 <<  " hit3: " << hit3 <<  " hit4: " << hit4 <<  " hit5: " << hit5 <<  " hit6: " << hit6 <<  " hit7: " << hit7 <<  " hit8: " << hit8 <<  " hit9: " << hit9 <<  " hit10: " << hit10 <<  " hit11: " << hit11 <<  std::endl;
        }

    }

}


//________________________________________________________________________________________________________________________________
void debugPrintOutlierMultiplicities(SDL::Event& event)
{
    SDL::trackCandidates& trackCandidatesInGPU = (*event.getTrackCandidates());
    SDL::tracklets& trackletsInGPU = (*event.getTracklets());
    SDL::triplets& tripletsInGPU = (*event.getTriplets());
    SDL::segments& segmentsInGPU = (*event.getSegments());
    SDL::miniDoublets& miniDoubletsInGPU = (*event.getMiniDoublets());
    SDL::hits& hitsInGPU = (*event.getHits());
    SDL::modules& modulesInGPU = (*event.getModules());
    int nTrackCandidates = 0;
    for (unsigned int idx = 0; idx <= *(modulesInGPU.nLowerModules); ++idx)
    {
        if (trackCandidatesInGPU.nTrackCandidates[idx] > 50000)
        {
            std::cout <<  " SDL::modulesInGPU->detIds[SDL::modulesInGPU->lowerModuleIndices[idx]]: " << modulesInGPU.detIds[modulesInGPU.lowerModuleIndices[idx]] <<  std::endl;
            std::cout <<  " idx: " << idx <<  " trackCandidatesInGPU.nTrackCandidates[idx]: " << trackCandidatesInGPU.nTrackCandidates[idx] <<  std::endl;
            std::cout <<  " idx: " << idx <<  " trackletsInGPU.nTracklets[idx]: " << trackletsInGPU.nTracklets[idx] <<  std::endl;
            std::cout <<  " idx: " << idx <<  " tripletsInGPU.nTriplets[idx]: " << tripletsInGPU.nTriplets[idx] <<  std::endl;
            unsigned int i = modulesInGPU.lowerModuleIndices[idx];
            std::cout <<  " idx: " << idx <<  " i: " << i <<  " segmentsInGPU.nSegments[i]: " << segmentsInGPU.nSegments[i] <<  std::endl;
            int nMD = miniDoubletsInGPU.nMDs[2*idx]+miniDoubletsInGPU.nMDs[2*idx+1] ;
            std::cout <<  " idx: " << idx <<  " nMD: " << nMD <<  std::endl;
            int nHits = 0;
            nHits += modulesInGPU.hitRanges[4*idx+1] - modulesInGPU.hitRanges[4*idx] + 1;       
            nHits += modulesInGPU.hitRanges[4*idx+3] - modulesInGPU.hitRanges[4*idx+2] + 1;
            std::cout <<  " idx: " << idx <<  " nHits: " << nHits <<  std::endl;
        }
    }
}
