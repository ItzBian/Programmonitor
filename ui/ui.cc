#include "../globals.hh"
#include "../imgui/imgui.h"

void AddProgram(const std::string& program) {
    if (std::find(monitoredPrograms.begin(), monitoredPrograms.end(), program) == monitoredPrograms.end()) {
        monitoredPrograms.push_back(program);
        programTimes[program] = 0; // initialize
    }
}

void RemoveProgram(const std::string& program) {
    monitoredPrograms.erase(std::remove(monitoredPrograms.begin(), monitoredPrograms.end(), program), monitoredPrograms.end());
    programTimes.erase(program); // remove
}

void Render() {
    // Live-Tracking
    ImGui::Begin("Aktives Programm");
    ImGui::Text("Aktuelles aktives Programm: %s", activeWindow.c_str());

    ImGui::Text("Log:");
    ImGui::BeginChild("Log", ImVec2(0, 100), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& entry : programTimes) {
        ImGui::Text("%s: %d Sekunden", entry.first.c_str(), entry.second);
    }
    ImGui::EndChild();
    ImGui::End();

    // add remove programs
    ImGui::Begin("Programme Hinzufuegen");

    ImGui::Text("Ueberwachte Programme (Drag & Drop moeglich):");
    for (const auto& program : monitoredPrograms) {
        bool isMainProgram = (mainProgram == program);
        if (ImGui::Selectable(program.c_str(), &isMainProgram)) {
            mainProgram = isMainProgram ? program : "";
        }
    }

    static char newProgram[128] = "";
    ImGui::InputText("Add Program (exe)", newProgram, IM_ARRAYSIZE(newProgram));
    if (ImGui::Button("Add")) {
        AddProgram(newProgram);
        memset(newProgram, 0, sizeof(newProgram));
    }

    if (ImGui::Button("Remove") && !mainProgram.empty()) {
        RemoveProgram(mainProgram);
        mainProgram.clear();
    }

    ImGui::End();
}
