// #include "virtual_device.h"

// VirtualDevice::VirtualDevice(int dimensions) : numOfDimensions(dimensions) {}

// void VirtualDevice::setAnchor(const std::string &selfId,
//                               const std::vector<std::pair<std::string, float>> &distances)
// {
//     auto it_res = virtualDevices.emplace(selfId, numOfDimensions);
//     VirtualDevice &self = it_res.first->second;

//     // Reset Kalman filter with initial position based on anchor distances
//     self.trilat.kf = KalmanFilter(numOfDimensions, 1.0f, 1.0f);

//     Matrix measurement(numOfDimensions, 1);

//     if (distances.size() == 0)
//     {
//         // ORIGIN
//         self.trilat.kf.init(measurement);
//         return;
//     }

//     // Build trilateration problem with known anchors
//     // (stub: assumes self.trilat has already solved and filled trilatSolution, null_space, alpha)

//     Matrix sol = self.trilat.trilatSolution.transpose();
//     Matrix null_space = self.trilat.null_space;
//     float alpha = self.trilat.alpha;

//     // Pick reference axis depending on step
//     Matrix ref(numOfDimensions, 1);
//     if (numOfDimensions >= distances.size())
//     {
//         ref[distances.size() - 1][0] = 1.0f;
//     }
//     else
//     {
//         // Free anchor → no bias
//         // Just take solution directly
//         self.trilat.kf.init(sol);
//         return;
//     }

//     // Resolve nullspace using axis bias
//     Matrix resolved = resolveNullspaceDirection(sol, null_space, alpha, ref);
//     self.trilat.kf.init(resolved);
// }

// void VirtualDevice::update(const std::string &selfId,
//                            const std::vector<std::pair<std::string, float>> &distances)
// {
//     VirtualDevice &self = virtualDevices.at(selfId);

//     // Here you'd typically fill self.trilat with new measurements
//     // Then compute trilatSolution and feed into Kalman filter update
//     // For now, let's just say:
//     Matrix sol = self.trilat.trilatSolution.transpose();
//     self.trilat.kf.init(sol); // if not initialized
//     // else self.trilat.kf.update(sol); <-- assuming you have such a method
// }

// //       |\      _,,,---,,_
// // ZZZzz /,`.-'`'    -.  ;-;;,_
// //      |,4-  ) )-,_. ,\ (  `'-'
// //     '---''(_/--'  `-'\_)  Felix Lee

// Matrix VirtualDevice::resolveNullspaceDirection(const Matrix &solution,
//                                                 const Matrix &null_space,
//                                                 float alpha,
//                                                 const Matrix &referenceDir)
// {
//     Matrix measurement = solution;

//     if (null_space.cols() != 0)
//     {
//         Matrix x = measurement;
//         for (int i = 0; i < measurement.rows(); i++)
//         {
//             x[i][0] -= referenceDir[i][0];
//         }
//         Matrix w = null_space * null_space.transpose() * x;
//         float w_len = w.norm();
//         if (w_len != 0.0f)
//         {
//             measurement = measurement - w * (alpha / w_len);
//         }
//         else
//         {
//             measurement = measurement + null_space.getColumn(0) * alpha;
//         }
//     }
//     return measurement;
// }
