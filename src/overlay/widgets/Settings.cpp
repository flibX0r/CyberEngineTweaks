#include <stdafx.h>

#include "Settings.h"

#include <CET.h>

#include <Utils.h>

Settings::Settings(Options& aOptions, LuaVM& aVm)
    : m_options(aOptions)
    , m_vm(aVm)
{
}

WidgetResult Settings::OnEnable()
{
    if (!m_enabled)
    {
        Load();
        m_enabled = true;
    }
    return m_enabled ? WidgetResult::ENABLED : WidgetResult::DISABLED;
}

WidgetResult Settings::OnDisable()
{
    WidgetResult result = WidgetResult::ENABLED;

    if (m_enabled)
    {
        m_vm.BlockDraw(m_madeChanges);
        const auto ret = UnsavedChangesPopup(
            m_openChangesModal,
            m_madeChanges,
            [this]{ Save(); },
            [this]{ Load(); });
        m_madeChanges = ret == THWUCPResult::CHANGED;
        m_vm.BlockDraw(m_madeChanges);

        m_enabled = m_madeChanges;
        if (ret == THWUCPResult::CANCEL)
        {
            CET::Get().GetOverlay().SetActiveWidget(WidgetID::SETTINGS);
            m_enabled = true;
            result = WidgetResult::CANCEL;
        }
    }
    if (!m_enabled)
    {
        // reset changes substates
        m_madeChanges = false;
    }

    if (result != WidgetResult::CANCEL)
        result = m_enabled ? WidgetResult::ENABLED : WidgetResult::DISABLED;

    return result;
}

void Settings::Update()
{
    const auto itemWidth = GetAlignedItemWidth(3);

    if (ImGui::Button("Load", ImVec2(itemWidth, 0)))
        Load();
    ImGui::SameLine();
    if (ImGui::Button("Save", ImVec2(itemWidth, 0)))
        Save();
    ImGui::SameLine();
    if (ImGui::Button("Defaults", ImVec2(itemWidth, 0)))
        ResetToDefaults();

    ImGui::Spacing();

    m_madeChanges = false;

    if (ImGui::CollapsingHeader("Patches"))
    {
        if (ImGui::BeginTable("##SETTINGS_PATCHES", 2, ImGuiTableFlags_Sortable | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Borders))
        {
            UpdateAndDrawSetting("AMD SMT Patch", "Requires restart to take effect.", m_patchAmdSmt, m_options.PatchAmdSmt);
            UpdateAndDrawSetting("Remove Pedestrians", "Requires restart to take effect.", m_patchRemovePedestrians, m_options.PatchRemovePedestrians);
            UpdateAndDrawSetting("Disable Async Compute", "Requires restart to take effect.", m_patchAsyncCompute, m_options.PatchAsyncCompute);
            UpdateAndDrawSetting("Disable Antialiasing", "Requires restart to take effect.", m_patchAntialiasing, m_options.PatchAntialiasing);
            UpdateAndDrawSetting("Skip Start Menu", "Requires restart to take effect.", m_patchSkipStartMenu, m_options.PatchSkipStartMenu);
            UpdateAndDrawSetting("Suppress Intro Movies", "Requires restart to take effect.", m_patchDisableIntroMovies, m_options.PatchDisableIntroMovies);
            UpdateAndDrawSetting("Disable Vignette", "Requires restart to take effect.", m_patchDisableVignette, m_options.PatchDisableVignette);
            UpdateAndDrawSetting("Disable Boundary Teleport", "Requires restart to take effect.", m_patchDisableBoundaryTeleport, m_options.PatchDisableBoundaryTeleport);
            UpdateAndDrawSetting("Disable V-Sync (Windows 7 only)", "Requires restart to take effect.", m_patchDisableWin7Vsync, m_options.PatchDisableWin7Vsync);
            UpdateAndDrawSetting("Fix Minimap Flicker", "Requires restart to take effect.", m_patchMinimapFlicker, m_options.PatchMinimapFlicker);

            ImGui::EndTable();
        }
    }

    if (ImGui::CollapsingHeader("Dev"))
    {
        if (ImGui::BeginTable("##SETTINGS_DEV", 2, ImGuiTableFlags_Sortable | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Borders))
        {
            UpdateAndDrawSetting("Draw ImGui Diagnostic Window", "Enables drawing of ImGui diagnostic window (useful for debugging ImGui issues).", m_options.DrawImGuiDiagnosticWindow, m_options.DrawImGuiDiagnosticWindow);
            UpdateAndDrawSetting("Remove Dead Bindings", "Removes all bindings from unloaded mods and all invalid ones (useful for debugging mod issues).", m_removeDeadBindings, m_options.RemoveDeadBindings);
            UpdateAndDrawSetting("Enable ImGui Assertions", "Enables ImGui assertions (useful for debugging ImGui issues).", m_enableImGuiAssertions, m_options.EnableImGuiAssertions);
            UpdateAndDrawSetting("Enable Debug Build", "Sets internal flags to imposter as debug build (requires restart to take effect).", m_patchEnableDebug, m_options.PatchEnableDebug);
            UpdateAndDrawSetting("Dump Game Options", "Requires restart to take effect.", m_dumpGameOptions, m_options.DumpGameOptions);

            ImGui::EndTable();
        }
    }
}

void Settings::Load()
{
    m_options.Load();

    m_patchRemovePedestrians = m_options.PatchRemovePedestrians;
    m_patchAsyncCompute = m_options.PatchAsyncCompute;
    m_patchAntialiasing = m_options.PatchAntialiasing;
    m_patchSkipStartMenu = m_options.PatchSkipStartMenu;
    m_patchAmdSmt = m_options.PatchAmdSmt;
    m_patchDisableIntroMovies = m_options.PatchDisableIntroMovies;
    m_patchDisableVignette = m_options.PatchDisableVignette;
    m_patchDisableBoundaryTeleport = m_options.PatchDisableBoundaryTeleport;
    m_patchDisableWin7Vsync = m_options.PatchDisableWin7Vsync;
    m_patchMinimapFlicker = m_options.PatchMinimapFlicker;

    m_removeDeadBindings = m_options.RemoveDeadBindings;
    m_enableImGuiAssertions = m_options.EnableImGuiAssertions;
    m_patchEnableDebug = m_options.PatchEnableDebug;
    m_dumpGameOptions = m_options.DumpGameOptions;
}

void Settings::Save() const
{
    m_options.PatchRemovePedestrians = m_patchRemovePedestrians;
    m_options.PatchAsyncCompute = m_patchAsyncCompute;
    m_options.PatchAntialiasing = m_patchAntialiasing;
    m_options.PatchSkipStartMenu = m_patchSkipStartMenu;
    m_options.PatchAmdSmt = m_patchAmdSmt;
    m_options.PatchDisableIntroMovies = m_patchDisableIntroMovies;
    m_options.PatchDisableVignette = m_patchDisableVignette;
    m_options.PatchDisableBoundaryTeleport = m_patchDisableBoundaryTeleport;
    m_options.PatchDisableWin7Vsync = m_patchDisableWin7Vsync;
    m_options.PatchMinimapFlicker = m_patchMinimapFlicker;

    m_options.RemoveDeadBindings = m_removeDeadBindings;
    m_options.EnableImGuiAssertions = m_enableImGuiAssertions;
    m_options.PatchEnableDebug = m_patchEnableDebug;
    m_options.DumpGameOptions = m_dumpGameOptions;

    m_options.Save();
}

void Settings::ResetToDefaults()
{
    m_options.ResetToDefaults();
    Load();
}

void Settings::UpdateAndDrawSetting(const std::string& acLabel, const std::string& acTooltip, bool& aCurrent, const bool& acSaved, float aOffsetX)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImVec4 curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    if (aCurrent != acSaved)
        curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

    ImGui::AlignTextToFramePadding();

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + aOffsetX);

    ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
    ImGui::Button(acLabel.c_str(), ImVec2(-FLT_MIN, 0));
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && !acTooltip.empty())
        ImGui::SetTooltip("%s", acTooltip.c_str());

    ImGui::TableNextColumn();

    if (ImGui::Button(aCurrent ? "ENABLED" : "DISABLED", ImVec2(-FLT_MIN, 0)))
        aCurrent = !aCurrent;
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", aCurrent ? "Click to disable this option." : "Click to enable this option.");

    ImGui::PopStyleColor();

    m_madeChanges |= aCurrent != acSaved;
}
