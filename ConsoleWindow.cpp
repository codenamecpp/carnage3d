#include "stdafx.h"
#include "ConsoleWindow.h"
#include "imgui.h"

ConsoleWindow gDebugConsoleWindow;

ConsoleWindow::ConsoleWindow() : DebugWindow("Debug Console")
{
    mCommands.push_back("HELP");
    mCommands.push_back("CLEAR");
}

void ConsoleWindow::DoUI(Timespan deltaTime)
{
    ImGui::SetNextWindowSize(ImVec2(520,600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(mWindowName, &mWindowShown))
    {
        ImGui::End();
        return;
    }

    ImGui::TextWrapped("Enter 'HELP' for help, press TAB to use text completion.");

    ImGui::Separator();

    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText

    // Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
    // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
    // You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
    // To use the clipper we could replace the 'for (int i = 0; i < Items.Size; i++)' loop with:
    //     ImGuiListClipper clipper(Items.Size);
    //     while (clipper.Step())
    //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
    // However, note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
    // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
    // and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
    // If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,1)); // Tighten spacing

    for (int i = 0; i < mItems.size(); i++)
    {
        const char* item = mItems[i].c_str();

        // Normally you would store more information in your item (e.g. make Items[] an array of structure, store color/type etc.)
        bool pop_color = false;
        if (strstr(item, "[error]"))            { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); pop_color = true; }
        else if (strncmp(item, "# ", 2) == 0)   { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.6f, 1.0f)); pop_color = true; }
        ImGui::TextUnformatted(item);
        if (pop_color)
        {
            ImGui::PopStyleColor();
        }
    }

    if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
    {
        ImGui::SetScrollHereY(1.0f);
    }
    ScrollToBottom = false;

    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::Separator();

    auto TextEditCallbackStub = [](ImGuiInputTextCallbackData* data) -> int
        {
            ConsoleWindow* this_ = (ConsoleWindow*) data->UserData;
            debug_assert(this_);
            return this_->TextEditCallback(data);
        };

    // Command-line
    bool reclaim_focus = false;
    if (ImGui::InputText("Input", mInputBuffer.c_str(), mInputBuffer.get_capacity() + 1, 
        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | 
        ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackResize, TextEditCallbackStub, (void*)this))
    {
        mInputBuffer.trim();
        if (!mInputBuffer.empty())
        {
            ExecCommand(mInputBuffer.c_str());
        }
        mInputBuffer.clear();
        reclaim_focus = true;
    }

    // Auto-focus on window apparition
    ImGui::SetItemDefaultFocus();
    if (reclaim_focus)
        ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

    ImGui::End();
}

void ConsoleWindow::AddLog(const char* fmt, ...)
{
    // FIXME-OPT
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
    buf[IM_ARRAYSIZE(buf)-1] = 0;
    va_end(args);
    mItems.push_back(buf);
}

void ConsoleWindow::ExecCommand(const char* command_line)
{
    AddLog("# %s\n", command_line);

    int historySize = mHistory.size();
    // Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
    mHistoryPos = -1;
    for (int i = historySize - 1; i >= 0; i--)
        if (::_stricmp(mHistory[i].c_str(), command_line) == 0)
        {
            mHistory.erase(mHistory.begin() + i);
            break;
        }
    mHistory.push_back(command_line);

    // todo: process command

    // On commad input, we scroll to bottom even if AutoScroll==false
    ScrollToBottom = true;
}

int ConsoleWindow::TextEditCallback(ImGuiInputTextCallbackData* data)
{
    switch (data->EventFlag)
    {
    case ImGuiInputTextFlags_CallbackCompletion:
        {
            // Example of TEXT COMPLETION

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

            // Build a list of candidates
            std::vector<const char*> candidates;
            for (int i = 0; i < mCommands.size(); i++)
                if (::_strnicmp(mCommands[i].c_str(), word_start, (int)(word_end-word_start)) == 0)
                {
                    candidates.push_back(mCommands[i].c_str());
                }

            int candidatesCount = candidates.size();
            if (candidatesCount == 0)
            {
                // No match
                AddLog("No match for \"%.*s\"!\n", (int)(word_end-word_start), word_start);
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

                // List matches
                AddLog("Possible matches:\n");
                for (int i = 0; i < candidatesCount; i++)
                {
                    AddLog("- %s\n", candidates[i]);
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
        }
    case ImGuiInputTextFlags_CallbackResize:
        {
            // Resize string callback
            // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
            data->Buf = mInputBuffer.c_str();
        }
    }
    return 0;
}