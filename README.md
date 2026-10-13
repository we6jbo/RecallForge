# RecallForge v0.3.9.1

RecallForge is a Qt6/C++ desktop application for study activities, learning progress, and writing workflows.

## Public release

The public repository provides the application source code and Flatpak packaging.

Private resources are not included:

- Exam-question banks and answer records
- Personal writing libraries and drafts
- Private Vibe instructions
- Genealogy records and personal research
- Credentials, settings, and publishing history

Optional features may be unavailable when their private backends or resources are not installed.

## Native build

Requirements: CMake 3.21+, C++17, and Qt6 Widgets.

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j2
    ./build/RecallForge

## Flatpak build

Install Flatpak Builder and the KDE 6.11 runtime and SDK.

    flatpak-builder --user --install --force-clean /tmp/recallforge-flatpak-build org.oneal.RecallForge.yml
    flatpak run org.oneal.RecallForge

The Flatpak manifest uses a local source directory. Prepare a clean public source tree before distributing builds.

## Version check

    flatpak run org.oneal.RecallForge --version-check

## Privacy and publication

RecallForge does not automatically authorize publication of private writing, social posts, or personal research. External operations require separate review and approval.

## License

See LICENSE and AI-DISCLOSURE.txt. Private and third-party material is not automatically covered by the project license.
