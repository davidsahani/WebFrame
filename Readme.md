## WebFrame: A web viewer built with ImGui and Miscrosoft WebView2 (Proof of Concept)

### Application Overview:

<img src="docs/screenshot.png">

---

## Prerequisites

Before building WebFrame, ensure the following tools are installed:

1. **[Visual Studio Community 2022](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=Community&rel=17)** *or* **[Visual Studio Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/)**
2. **Desktop Development with C++** workload (Install via the Visual Studio Installer).
3. **Windows 11 SDK** (version 10.0.22621.0 or later). Install via the Visual Studio Installer if not already available.

---

## Dependency Installation

Install dependencies using one of the following methods:

### Method 1: Using the Provided Script

1. **Open a Command Prompt** and navigate to the project root.

2. **Run the batch Script:**
   ```bash
   install_dependencies.bat
   ```

### Method 2: Manual Installation

1. Clone the vcpkg repository:
   ```bash
   git clone https://github.com/microsoft/vcpkg.git
   ```
2. Bootstrap vcpkg:
   ```bash
   .\vcpkg\bootstrap-vcpkg.bat
   ```
3. Install dependencies:
   ```bash
   .\vcpkg\vcpkg install --triplet x64-windows
   ```

---

## Building the Project

Follow these steps to build WebFrame using VS Code:

1. **Open the Project in VS Code.**
2. **Install the [CMake Tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools).**
3. **Configure & Build:**
   - Open the command palette (`Ctrl+Shift+P`) and run **CMake: Configure**.
   - After configuration, run **CMake: Build**.

---

## Troubleshooting

- **Dependency Issues:**  
  If you encounter issues related to missing dependencies, verify that the Visual Studio workload and Windows SDK are properly installed.

- **Build Errors:**  
  Ensure that the vcpkg dependencies are installed correctly. You may need to re-run the dependency installation steps or verify your CMake configuration.

- **Running in VSCode:**  
  If VSCode does not detect CMake automatically, try reloading the window or checking the extension settings.
