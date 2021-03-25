#include "stdafx.h"
#include "ConsoleWindow.h"
#include "imgui.h"
#include "Console.h"
#include "ConsoleVar.h"

ConsoleWindow gDebugConsoleWindow;

ConsoleWindow::ConsoleWindow() : DebugWindow("Debug Console")
{
}

void ConsoleWindow::DoUI(ImGuiIO& imguiContext)
{
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

    ImGui::SetNextWindowBgAlpha(0.7f);
    if (!ImGui::Begin("Console", &mWindowShown, windowFlags))
    {
        ImGui::End();
        return;
    }

    if (ImGui::IsWindowAppearing())
    {
        ImVec2 distance { 10.0f, 10.0f };
        ImVec2 windowSize { 
            imguiContext.DisplaySize.x - distance.x * 2.0f, 
            imguiContext.DisplaySize.y * 0.45f - distance.y * 2.0f};
        ImVec2 windowPos { distance.x, distance.y };
        ImGui::SetWindowSize(windowSize);
        ImGui::SetWindowPos(windowPos);
    }

    ImGui::Separator();

    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, 
        ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoBackground);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,1));

    for (const ConsoleLine& currentLine: gConsole.mLines)
    {
        const char* item = currentLine.mString.c_str();

        bool pop_color = false;
        if (currentLine.mMessageCategory == eLogMessage_Error) 
        { 
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); 
            pop_color = true; 
        }
        if (currentLine.mMessageCategory == eLogMessage_Warning) 
        { 
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); 
            pop_color = true; 
        }
        else if (currentLine.mMessageCategory == eLogMessage_Debug)
        { 
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f)); 
            pop_color = true; 
        }
        ImGui::TextUnformatted(item);
        if (pop_color)
        {
            ImGui::PopStyleColor();
        }
    }

    if (mScrollToBottom || (mAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
    {
        ImGui::SetScrollHereY(1.0f);
    }
    mScrollToBottom = false;

    ImGui::PopStyleVar();
    ImGui::EndChild();

    ImGui::Separator();

    auto TextEditCallbackStub = [](ImGuiInputTextCallbackData* data) -> int
    {
        ConsoleWindow* this_ = (ConsoleWindow*) data->UserData;
        debug_assert(this_);
        return this_->TextEditCallback(data);
    };

    // command-line
    bool reclaim_focus = false;
    if (ImGui::InputText("Input", (char*)mInputString.c_str(), mInputString.capacity() + 1, 
        ImGuiInputTextFlags_EnterReturnsTrue | 
        ImGuiInputTextFlags_CallbackCompletion | 
        ImGuiInputTextFlags_CallbackHistory | 
        ImGuiInputTextFlags_CallbackResize | 
        ImGuiInputTextFlags_CallbackCharFilter, TextEditCallbackStub, (void*)this))
    {
        Exec();
        reclaim_focus = true;
    }

    if (!ImGui::IsAnyItemActive())
    {
        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
        {
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
        }
        else
        {
            ImGui::SetKeyboardFocusHere(0); // Auto focus previous widget
        }
    }
    ImGui::End();
}

int ConsoleWindow::TextEditCallback(ImGuiInputTextCallbackData* data)
{
    switch (data->EventFlag)
    {
    case ImGuiInputTextFlags_CallbackCompletion:
        {
            // Locate beginning of current word
            const char* word_end = data->Buf + data->CursorPos;
            const char* word_start = word_end;
            while (word_start > data->Buf)
            {
                const char c = word_start[-1];
                if (c == ' ' || c == '\t' || c == ',' || c == ';')
                    break;

                word_start--;
            }

            if (word_end == word_start)
                break;

            // build a list of candidates
            std::vector<const char*> candidates;

            // cvars
            for (Cvar* currCvar: gConsole.mCvarsList)
            {
                if (cxx_strnicmp(currCvar->mName.c_str(), word_start, (int)(word_end-word_start)) == 0)
                {
                    candidates.push_back(currCvar->mName.c_str());
                }
            }

            int candidatesCount = candidates.size();
            if (candidatesCount == 0)
            {
                // no match
            }
            else if (candidatesCount == 1)
            {
                // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
                data->DeleteChars((int)(word_start-data->Buf), (int)(word_end-word_start));
                data->InsertChars(data->CursorPos, candidates[0]);
                data->InsertChars(data->CursorPos, " ");
            }
            else
            {
                // Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
                int match_len = (int)(word_end - word_start);
                for (;;)
                {
                    int c = 0;
                    bool all_candidates_matches = true;
                    for (int i = 0; i < candidatesCount && all_candidates_matches; i++)
                        if (i == 0)
                            c = toupper(candidates[i][match_len]);
                        else if (c == 0 || c != toupper(candidates[i][match_len]))
                            all_candidates_matches = false;
                    if (!all_candidates_matches)
                        break;
                    match_len++;
                }

                if (match_len > 0)
                {
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end-word_start));
                    data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                }
                gConsole.LogMessage(eLogMessage_Debug, "");
                // List matches
                for (int i = 0; i < candidatesCount; i++)
                {
                    gConsole.LogMessage(eLogMessage_Debug, "- %s\n", candidates[i]);
                }
            }

            break;
        }
    case ImGuiInputTextFlags_CallbackHistory:
        {
            int historySize = mHistory.size();
            // Example of HISTORY
            const int prev_history_pos = mHistoryPos;
            if (data->EventKey == ImGuiKey_UpArrow)
            {
                if (mHistoryPos == -1)
                {
                    mHistoryPos = historySize - 1;
                }
                else if (mHistoryPos > 0)
                {
                    mHistoryPos--;
                }
            }
            else if (data->EventKey == ImGuiKey_DownArrow)
            {
                if (mHistoryPos != -1)
                {
                    if (++mHistoryPos >= historySize)
                    {
                        mHistoryPos = -1;
                    }
                }
            }

            // A better implementation would preserve the data on the current input line along with cursor position.
            if (prev_history_pos != mHistoryPos)
            {
                const char* history_str = (mHistoryPos >= 0) ? mHistory[mHistoryPos].c_str() : "";
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, history_str);
            }
            break;
        }
    case ImGuiInputTextFlags_CallbackResize:
        {
            mInputString.resize(data->BufTextLen);

            data->Buf = (char*)mInputString.c_str();
            break;
        }
    }
    return 0;
}

void ConsoleWindow::Exec()
{
    if (!mInputString.empty())
    {
        cxx::trim(mInputString);
    }

    if (mInputString.empty())
        return;

    gConsole.ExecuteCommands(mInputString.c_str());
    MoveInputToHistory();
}

void ConsoleWindow::MoveInputToHistory()
{
    const int MaxHistoryEntries = 6;
    if (mHistory.size() >= MaxHistoryEntries)
    {
        mHistory.pop_front();
    }
    mHistory.emplace_back(std::move(mInputString));
}
