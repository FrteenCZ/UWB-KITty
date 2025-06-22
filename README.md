# UWB-KITty
This project is designed to deliver high-precision indoor tracking using UWB technology, with a focus on usability and modularity.


## Features
- 🧭 Real-time trilateration using UWB anchors and tags
- 🐾 Accurate position estimation indoors
- 📡 Flexible architecture (ROS / custom middleware)
- 🛠️ Calibration tools and visualization


## 📂 Folder Structure

```

UWB-KITty/
├── docs/               # Diagrams, specifications, documentation
├── hardware/           # Schematics, anchor/tag configs
├── src/                # Core source code (localization, filtering, etc.)
├── sim/                # Simulation tools and configs
├── scripts/            # Helper scripts (e.g., setup, calibration)
├── tests/              # Unit tests and mock environments
├── README.md
└── LICENSE

````


## 🚀 Getting Started

```bash
git clone https://github.com/yourusername/UWB-KITty.git
cd UWB-KITty
# Installation / setup instructions
````

## 📜 Licenses and Acknowledgments

This project uses the [**DW1000 driver** by thotro](https://github.com/thotro/arduino-dw1000), licensed under the [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0).

My own code is licensed under the MIT License.

Please refer to the respective LICENSE files for full terms.


