#include <stdlib.h>

#include "cli.hpp"
#include "main.hpp"
#include "Imgui/misc/cpp/imgui_stdlib.h"

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <clocale>
#include <locale>
#include <codecvt>

static const WORD MAX_CONSOLE_LINES = 500;
bool RedirectIOToConsole()
{
    bool result = true;
    FILE* fp;

    // Redirect STDIN if the console has an input handle
    if (GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE)
        if (freopen_s(&fp, "CONIN$", "r", stdin) != 0)
            result = false;
        else
            setvbuf(stdin, NULL, _IONBF, 0);

    // Redirect STDOUT if the console has an output handle
    if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE)
        if (freopen_s(&fp, "CONOUT$", "w", stdout) != 0)
            result = false;
        else
            setvbuf(stdout, NULL, _IONBF, 0);

    // Redirect STDERR if the console has an error handle
    if (GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE)
        if (freopen_s(&fp, "CONOUT$", "w", stderr) != 0)
            result = false;
        else
            setvbuf(stderr, NULL, _IONBF, 0);

    // Make C++ standard streams point to console as well.
    std::ios::sync_with_stdio(true);

    // Clear the error state for each of the C++ standard streams.
    std::wcout.clear();
    std::cout.clear();
    std::wcerr.clear();
    std::cerr.clear();
    std::wcin.clear();
    std::cin.clear();

    return result;
}

bool ReleaseConsole()
{
    bool result = true;
    FILE* fp;

    // Just to be safe, redirect standard IO to NUL before releasing.

    // Redirect STDIN to NUL
    if (freopen_s(&fp, "NUL:", "r", stdin) != 0)
        result = false;
    else
        setvbuf(stdin, NULL, _IONBF, 0);

    // Redirect STDOUT to NUL
    if (freopen_s(&fp, "NUL:", "w", stdout) != 0)
        result = false;
    else
        setvbuf(stdout, NULL, _IONBF, 0);

    // Redirect STDERR to NUL
    if (freopen_s(&fp, "NUL:", "w", stderr) != 0)
        result = false;
    else
        setvbuf(stderr, NULL, _IONBF, 0);

    // Detach from console
    if (!FreeConsole())
        result = false;

    return result;
}

void AdjustConsoleBuffer(int16_t minLength)
{
    // Set the screen buffer to be big enough to scroll some text
    CONSOLE_SCREEN_BUFFER_INFO conInfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conInfo);
    if (conInfo.dwSize.Y < minLength)
        conInfo.dwSize.Y = minLength;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), conInfo.dwSize);
}

bool CreateNewConsole(int16_t minLength)
{
    bool result = false;

    // Release any current console and redirect IO to NUL
    ReleaseConsole();

    // Attempt to create new console
    if (AllocConsole())
    {
        AdjustConsoleBuffer(minLength);
        result = RedirectIOToConsole();
    }

    return result;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nShowCmd) {
	LPWSTR *args;
	int nArgs;
	args = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (nArgs < 2) {
		ImFrame::Run("uraymeiviar", "Chia Plotter", [] (const auto & params) { 
			return std::make_unique<gui::MainApp>(params); 
		});
	}
	else {
		AttachConsole(ATTACH_PARENT_PROCESS );
		HWND consoleWnd = GetConsoleWindow();
		bool ownConsole = false;
		if (consoleWnd == NULL)
		{
			ownConsole = CreateNewConsole(1024);
		}
		else {
			RedirectIOToConsole();
		}
		std::cout << std::endl;
		if (nArgs >= 1)
		{
			std::filesystem::path exePath(args[0]);
			if (nArgs >= 2) {
				std::wstring command = std::wstring(args[1]);
				if (lowercase(command) == L"create") {
					if (nArgs < 4) {
						std::cout << "Usage "<< exePath.filename().string() <<" create <<options>>" << std::endl;
						std::cout << "options :" << std::endl;
						std::cout << "  -f  --farmkey     : farmer public key in hex (48 bytes)" << std::endl;
						std::cout << "                      if not specified, plot id must be given" << std::endl;
						std::cout << "  -p  --poolkey     : pool public key in hex (48 bytes)" << std::endl;
						std::cout << "                      if not specified, plot id must be given" << std::endl;
						std::cout << "  -d  --dest        : plot destination path" << std::endl;
						std::cout << "                      if not specified will use current path" << std::endl;
						std::cout << "  -t  --temp        : plotting temporary path" << std::endl;
						std::cout << "                      if not specified will use current path" << std::endl;
						std::cout << "  -l  --temp2       : plotting final temporary path" << std::endl;
						std::cout << "                      if not specified will use temp path" << std::endl;
						std::cout << "  -o  --output      : output file name" << std::endl;
						std::cout << "                      if not specified will be named plot-[k]-[date]-[id].plot" << std::endl;
						std::cout << "  -m  --memo        : plot memo bytes in hex" << std::endl;
						std::cout << "                      if not specified will generated from farm and pool public key" << std::endl;
						std::cout << "  -i  --id          : plot id in hex" << std::endl;
						std::cout << "                      if specified farmkey and pool key will be ignored" << std::endl;
						std::cout << "  -k  --size        : plot size               (default 32   )" << std::endl;
						std::cout << "  -r  --threads     : cpu threads to use      (default 2    )" << std::endl;
						std::cout << "  -b  --buckets     : number of buckets       (default 128  )" << std::endl;
						std::cout << "  -m  --mem         : max memory buffer in MB (default 4608 )" << std::endl;
						std::cout << "  -s  --stripes     : stripes count           (default 65536)" << std::endl;
						std::cout << "  -n  --no-bitfield : use bitfield            (default set  )" << std::endl << std::endl;
						std::cout << " common usage example :" << std::endl;
						std::cout << exePath.filename().string() << " create -f b6cce9c6ff637f1dc9726f5db64776096fdb4101d673afc4e27ec71f0f9a859b2f1d661c92f3b8e6932a3f7634bc4c12 -p 86e2a9cf0b409c8ca7258f03ef7698565658a17f6f7dd9e9b0ac9be6ca3891ac09fa8468951f24879c00870e88fa66bb -d D:\\chia-plots -t C:\\chia-temp" << std::endl << std::endl;
						std::cout << "this command will create default 100GB k-32 plot to D:\\chia-plots\\ and use C:\\chia-plots as temporary directory, plot id, memo, and filename will be generated from farm and plot public key, its recommend to use buckets, k-size and stripes to default value" << std::endl;
					}
					else {
						std::string farmkey;
						std::string poolkey;
						std::wstring dest;
						std::wstring temp;
						std::wstring temp2;
						std::wstring filename;
						std::string memo;
						std::string id;
						int ksize = 32;
						int nthreads = 2;
						int buckets = 128;
						int mem = 3390;
						int stripes = 65556;
						bool bitfield = true;

						std::string lastArg = "";
						for (int i = 2; i < nArgs; i++) {
							if (lastArg.empty()) {
								lastArg = lowercase(ws2s(std::wstring(args[i])));
								if (lastArg == "-n" || lastArg == "--no-bitfield") {
									bitfield = false;
								}
							}
							else {
								if (lastArg == "-f" || lastArg == "--farmkey") {
									farmkey = lowercase(ws2s(std::wstring(args[i])));
								}
								else if (lastArg == "-p" || lastArg == "--poolkey") {
									poolkey = lowercase(ws2s(std::wstring(args[i])));
								}
								else if (lastArg == "-d" || lastArg == "--dest") {
									dest = std::wstring(args[i]);
								}
								else if (lastArg == "-t" || lastArg == "--temp") {
									temp = std::wstring(args[i]);
								}
								else if (lastArg == "-l" || lastArg == "--temp2") {
									temp2 = std::wstring(args[i]);
								}
								else if (lastArg == "-o" || lastArg == "--output") {
									filename = std::wstring(args[i]);
								}
								else if (lastArg == "-m" || lastArg == "--memo") {
									memo = lowercase(ws2s(std::wstring(args[i])));
								}
								else if (lastArg == "-i" || lastArg == "--id") {
									id = lowercase(ws2s(std::wstring(args[i])));
								}
								else if (lastArg == "-k" || lastArg == "--size") {
									std::wstring ksizeStr = std::wstring(args[i]);
									try {
										ksize = std::stoi(ksizeStr);
									}
									catch (...) {
										std::cout << "parsing error on plot size argument, revert back to default 32";
										ksize = 32;
									}
								}
								else if (lastArg == "-r" || lastArg == "--threads") {
									std::wstring valStr = std::wstring(args[i]);
									try {
										nthreads = std::stoi(valStr);
									}
									catch (...) {
										std::cout << "parsing error on thread count argument, revert back to default 2";
										nthreads = 2;
									}
								}
								else if (lastArg == "-b" || lastArg == "--buckets") {
									std::wstring valStr = std::wstring(args[i]);
									try {
										buckets = std::stoi(valStr);
									}
									catch (...) {
										std::cout << "parsing error on buckets count argument, revert back to default 128";
										buckets = 128;
									}
								}
								else if (lastArg == "-m" || lastArg == "--mem") {
									std::wstring valStr = std::wstring(args[i]);
									try {
										mem = std::stoi(valStr);
									}
									catch (...) {
										std::cout << "parsing error memory size argument, revert back to default 3390 MB";
										mem = 3390;
									}
								}
								else if (lastArg == "-s" || lastArg == "--stripes") {
									std::wstring valStr = std::wstring(args[i]);
									try {
										stripes = std::stoi(valStr);
									}
									catch (...) {
										std::cout << "parsing error stripes count argument, revert back to default 65536";
										stripes = 65536;
									}
								}
								else if (lastArg == "-n" || lastArg == "--no-bitfield") {
									std::wstring valStr = std::wstring(args[i]);
									if (valStr == L"1" || lowercase(valStr) == L"true" || lowercase(valStr) == L"on") {
										bitfield = true;
									}
									else {
										bitfield = false;
									}
								}
								else {
									std::cout << L"ignored unknown argument "<< args[i] << std::endl;
								}
							}
						}

						if (farmkey.empty() && id.empty()) {
							std::cerr << "error: either farmkey or id need to be specified";
						}
						if (poolkey.empty() && id.empty()) {
							std::cerr << "error: either poolkey or id need to be specified";
						}
						if (dest.empty()) {
							std::cout << "destination path will be using current directory" << std::endl;
						}
						else {
							std::wcout << L"destination path = " << dest << std::endl;
						}
						if (temp.empty()) {
							std::cout << "temp path will be using current directory" << std::endl;
						}
						else {
							std::wcout << L"temp path = " << temp << std::endl;
						}
						if (temp2.empty()) {
							std::cout << "temp2 path will use temp path" << std::endl;
							temp2 = temp;
						}
						else {
							std::wcout << L"temp2 path = " << temp2 << std::endl;
						}
						if (filename.empty()) {
							std::cout << "will use default plot filename" << std::endl;
						}
						else {
							std::wcout << L"plot file name = " << filename << std::endl;
						}
						if (memo.empty()) {
							std::cout << "will auto generate default memo for farming" << std::endl;
						}
						else {
							std::cout << L"memo content = " << memo << std::endl;
						}
						if (id.empty()) {
							std::cout << "will auto generate default plot id for farming" << std::endl;
						}
						else {
							std::cout << L"plot id = " << id << std::endl;
						}
						if (bitfield) {
							std::cout << "will generate plot with bitfield" << std::endl;
						}
						else {
							std::cout << "will generate plot without bitfield" << std::endl;
						}

						std::cout << "plot ksize    = " << std::to_string(ksize) << std::endl;
						std::cout << "threads count = " << std::to_string(nthreads) << std::endl;
						std::cout << "buckets count = " << std::to_string(buckets) << std::endl;
						std::cout << "max memsize   = " << std::to_string(mem) << " MB" << std::endl;
						std::cout << "stripes count = " << std::to_string(stripes) << std::endl;
					}
				}
				else if (lowercase(command) == L"proof") {
					if (nArgs < 4) {
						std::cout << "Usage "<< exePath.filename().string() <<" proof <filepath> <challenge>" << std::endl;
						std::cout << "   filepath  : path to file to check" << std::endl;
						std::cout << "   challenge : multiple of 8 bytes in hex" << std::endl;
					}
					else {
						std::filesystem::path targetPath(args[2]);
						if (std::filesystem::exists(targetPath) && std::filesystem::is_regular_file(targetPath)) {
							std::string proof = ws2s(std::wstring(args[3]));
							cli_proof(proof,targetPath.wstring());
						}
						else {
							std::cerr << "file not exist" << std::endl;
						}
					}
				}
				else if (lowercase(command) == L"verify") {
					if (nArgs < 5) {
						std::cout << "Usage "<< exePath.filename().string() <<" verify <id> <proof> <challenge>" << std::endl;
						std::cout << "   id        : 32 bytes ID in hex" << std::endl;
						std::cout << "   proof     : 32 bytes proof in hex" << std::endl;
						std::cout << "   challenge : multiple of 8 bytes in hex" << std::endl;
					}
					else {
						std::string id = ws2s(std::wstring(args[2]));
						std::string proof = ws2s(std::wstring(args[3]));
						std::string challenge = ws2s(std::wstring(args[4]));
						cli_verify(id, proof, challenge);
					}
				}
				else if (lowercase(command) == L"check") {
					if (nArgs < 3) {
						std::cout << "Usage "<< exePath.filename().string() <<" check <filepath> [iteration]" << std::endl;
						std::cout << "   filepath  : path to file to check" << std::endl;
						std::cout << "   iteration : number of iteration to perform (default:100)" << std::endl;
					}
					else {
						uint32_t iteration = 100;
						std::filesystem::path targetPath(args[2]);
						if (std::filesystem::exists(targetPath) && std::filesystem::is_regular_file(targetPath)) {
							if (nArgs >= 4) {
								std::wstring iterationStr = args[3];
								try {
									iteration = std::stoi(iterationStr);
								}
								catch (...) {
									iteration = 100;
								}
							}
							cli_check(iteration,targetPath.wstring());
						}
						else {
							std::cerr << "file not exist" << std::endl;
						}
					}
				}
				else {
					nArgs = 1;
				}
			}
			if (nArgs <= 1) {
				std::cout << "Usage "<< exePath.filename().string() <<" <command> <args>" << std::endl;
				std::cout << "command options:" << std::endl;
				std::cout << "    create " << std::endl;
				std::cout << "    proof " << std::endl;
				std::cout << "    verify " << std::endl;
				std::cout << "    check " << std::endl;
			}			
		}
		
		if (ownConsole) {
			ReleaseConsole();
		}
		else {
			std::cout << "done, press any key to exit" << std::endl;
			FreeConsole();
		}
		
		return 0;
	}
	return 1;
}

constexpr auto ColorFromBytes = [](uint8_t r, uint8_t g, uint8_t b)
{
	return ImVec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f);
};

const ImVec4 bgColor           = ColorFromBytes(4, 16, 8);
const ImVec4 lightBgColor      = ColorFromBytes(40, 65, 50);
const ImVec4 veryLightBgColor  = ColorFromBytes(70, 115, 90);

const ImVec4 panelColor        = ColorFromBytes(80, 140, 110);
const ImVec4 panelHoverColor   = ColorFromBytes(30, 215, 140);
const ImVec4 panelActiveColor  = ColorFromBytes(0, 200, 120);

const ImVec4 textColor         = ColorFromBytes(200, 255, 210);
const ImVec4 textDisabledColor = ColorFromBytes(96, 96, 96);
const ImVec4 borderColor       = ColorFromBytes(30, 48, 40);

void Style()
{
	auto& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	colors[ImGuiCol_Text]                 = textColor;
	colors[ImGuiCol_TextDisabled]         = textDisabledColor;
	colors[ImGuiCol_TextSelectedBg]       = panelActiveColor;
	colors[ImGuiCol_WindowBg]             = bgColor;
	colors[ImGuiCol_ChildBg]              = bgColor;
	colors[ImGuiCol_PopupBg]              = lightBgColor;
	colors[ImGuiCol_Border]               = borderColor;
	colors[ImGuiCol_BorderShadow]         = borderColor;
	colors[ImGuiCol_FrameBg]              = lightBgColor;
	colors[ImGuiCol_FrameBgHovered]       = panelHoverColor;
	colors[ImGuiCol_FrameBgActive]        = panelActiveColor;
	colors[ImGuiCol_TitleBg]              = bgColor;
	colors[ImGuiCol_TitleBgActive]        = bgColor;
	colors[ImGuiCol_TitleBgCollapsed]     = bgColor;
	colors[ImGuiCol_MenuBarBg]            = panelColor;
	colors[ImGuiCol_ScrollbarBg]          = borderColor;
	colors[ImGuiCol_ScrollbarGrab]        = lightBgColor;
	colors[ImGuiCol_ScrollbarGrabHovered] = veryLightBgColor;
	colors[ImGuiCol_ScrollbarGrabActive]  = panelActiveColor;
	colors[ImGuiCol_CheckMark]            = textColor;
	colors[ImGuiCol_SliderGrab]           = panelHoverColor;
	colors[ImGuiCol_SliderGrabActive]     = panelActiveColor;
	colors[ImGuiCol_Button]               = lightBgColor;
	colors[ImGuiCol_ButtonHovered]        = veryLightBgColor;
	colors[ImGuiCol_ButtonActive]         = panelHoverColor;
	colors[ImGuiCol_Header]               = panelColor;
	colors[ImGuiCol_HeaderHovered]        = panelHoverColor;
	colors[ImGuiCol_HeaderActive]         = panelActiveColor;
	colors[ImGuiCol_Separator]            = borderColor;
	colors[ImGuiCol_SeparatorHovered]     = borderColor;
	colors[ImGuiCol_SeparatorActive]      = borderColor;
	colors[ImGuiCol_ResizeGrip]           = bgColor;
	colors[ImGuiCol_ResizeGripHovered]    = lightBgColor;
	colors[ImGuiCol_ResizeGripActive]     = veryLightBgColor;
	colors[ImGuiCol_PlotLines]            = panelActiveColor;
	colors[ImGuiCol_PlotLinesHovered]     = panelHoverColor;
	colors[ImGuiCol_PlotHistogram]        = panelActiveColor;
	colors[ImGuiCol_PlotHistogramHovered] = panelHoverColor;
	colors[ImGuiCol_DragDropTarget]       = bgColor;
	colors[ImGuiCol_NavHighlight]         = bgColor;
	colors[ImGuiCol_Tab]                  = bgColor;
	colors[ImGuiCol_TabActive]            = panelActiveColor;
	colors[ImGuiCol_TabUnfocused]         = bgColor;
	colors[ImGuiCol_TabUnfocusedActive]   = panelActiveColor;
	colors[ImGuiCol_TabHovered]           = panelHoverColor;
	colors[ImGuiCol_TableHeaderBg]        = lightBgColor;

	style.WindowRounding    = 8.0f;
	style.ChildRounding     = 4.0f;
	style.FrameRounding     = 7.0f;
	style.GrabRounding      = 4.0f;
	style.PopupRounding     = 4.0f;
	style.ScrollbarRounding = 4.0f;
	style.TabRounding       = 0.0f;
	style.FramePadding		= ImVec2(6.0f,3.0f);
	style.WindowPadding     = ImVec2(4.0f,0.0f);
	style.ItemSpacing		= ImVec2(2.0f,4.0f);
	style.AntiAliasedLines = true;
	style.AntiAliasedFill = true;
	style.TabRounding       = 0.0f;
}

namespace gui {

	void tooltiipText(std::string txt, float padding_x = 4.0f, float padding_y = 2.0f) {
		ImVec2 sz = ImGui::CalcTextSize(txt.c_str());
		ImVec2 cursor = ImGui::GetCursorPos();
		ImGui::InvisibleButton("##padded-text", ImVec2(sz.x + padding_x * 2, sz.y + padding_y * 2));    // ImVec2 operators require imgui_internal.h include and -DIMGUI_DEFINE_MATH_OPERATORS=1
		ImVec2 final_cursor_pos = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2(cursor.x + padding_x, cursor.y + padding_y));
		ImGui::Text(txt.c_str());
		ImGui::SetCursorPos(final_cursor_pos);
	}

	const ImGuiTableFlags tableFlag = 
		ImGuiTableFlags_Resizable|
		ImGuiTableFlags_SizingFixedFit|
		ImGuiTableFlags_NoBordersInBody|
		ImGuiTableFlags_NoBordersInBodyUntilResize;

	MainApp::MainApp(GLFWwindow* window) : ImFrame::ImApp(window) {
		glfwSetWindowSize(this->GetWindow(),800,400);
	}

	void MainApp::OnUpdate() {
		Style();
		glfwGetWindowPos(this->GetWindow(),&wx,&wy);
		glfwGetWindowSize(this->GetWindow(),&ww,&wh);
		ImGui::SetNextWindowPos(ImVec2((float)wx,(float)wy));
		ImGui::SetNextWindowSize(ImVec2((float)(ww),(float)(wh)));
		ImGui::SetNextWindowSizeConstraints(ImVec2((float)ww,(float)wh),ImVec2(float(ww),float(wh)));
		if (ImGui::Begin("gfg",nullptr,ImGuiWindowFlags_NoDecoration)) {
			if (ImGui::BeginTabBar("MainTab")) {
				if(ImGui::BeginTabItem("Tools")){
					this->activeTab = 0;
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Stats")) {
					this->activeTab = 1;
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("System")) {
					this->activeTab = 2;
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Help")) {
					this->activeTab = 3;
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			if (this->activeTab == 0) {
				this->toolPage();
			}
			else if (this->activeTab == 1) {
				this->statPage();
			}
			else if(this->activeTab == 2){
				this->systemPage();
			}
			else if (this->activeTab == 3) {
				this->helpPage();
			}
		}
		ImGui::End();
	}

	void MainApp::toolPage() {
		ImGui::PushID("toolpage");
		
		if(ImGui::BeginTable("toolTable",3,tableFlag)){
			ImGui::TableSetupScrollFreeze(1, 1);
			ImGui::TableSetupColumn("Create Job",ImGuiTableColumnFlags_WidthFixed,340.0f);
			ImGui::TableSetupColumn("Active Job",ImGuiTableColumnFlags_WidthFixed,230.0f);
			ImGui::TableSetupColumn("Job Status",ImGuiTableColumnFlags_WidthStretch,240);
			ImGui::TableHeadersRow();			

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (ImGui::BeginChild("col1", ImVec2(0.0f, 0.0f))) {
				if (ImGui::CollapsingHeader("Create Plot")) {
					ImGui::Indent(20.0f);
					this->createPlotDialog();
					ImGui::Unindent(20.0f);
				}
				if (ImGui::CollapsingHeader("Check Plot")) {
					ImGui::Text("Under development");
				}
				ImGui::EndChild();
			}

			ImGui::TableSetColumnIndex(1);
			if (JobManager::getInstance().countJob() < 1) {
				ImGui::Text("No Active Job, create from left panel");
			}
			else {
				if (ImGui::BeginChild("col2", ImVec2(0.0f, 0.0f))) {
					for(auto it = JobManager::getInstance().jobIteratorBegin() ; it != JobManager::getInstance().jobIteratorEnd() ; it++){
						ImGui::PushID((const void*)it->get());
						ImVec2 cursorBegin = ImGui::GetCursorPos();
						if (*it == JobManager::getInstance().getSelectedJob()) {
							ImGui::GetStyle().Colors[ImGuiCol_Border] = textColor;
						}
						else {
							ImGui::GetStyle().Colors[ImGuiCol_Border] = lightBgColor;
						}
						ImGui::BeginGroupPanel();
						(*it)->drawItemWidget();
						ImVec2 cursorEnd = ImGui::GetCursorPos();
						float invisWidth = ImGui::GetContentRegionAvailWidth();
						ImGui::SetCursorPos(cursorBegin);
						if(ImGui::InvisibleButton("select", ImVec2(ImGui::GetContentRegionAvailWidth(), cursorEnd.y - cursorBegin.y))){
							JobManager::getInstance().setSelectedJob(*it);
						}					
						ImGui::EndGroupPanel();
						ImGui::GetStyle().Colors[ImGuiCol_Border] = borderColor;
						ImGui::PopID();
					}
					ImGui::EndChild();
				}
			}

			ImGui::TableSetColumnIndex(2);
			std::shared_ptr<Job> job = JobManager::getInstance().getSelectedJob();
			if (job) {
				if (ImGui::BeginChild(job->getTitle().c_str())) {
					job->drawStatusWidget();
					ImGui::EndChild();
				}
			}

			ImGui::EndTable();
		}
		ImGui::PopID();
	}

	void MainApp::statPage() {}

	void MainApp::systemPage() {}

	void MainApp::helpPage() {}

	void MainApp::createPlotDialog() {
		JobCreatePlotParam* jobPram = JobCreatePlot::drawUI();

		float fieldWidth = ImGui::GetWindowContentRegionWidth();

		ImGui::Text("Job Name");
		ImGui::SameLine(90.0f);
		ImGui::PushItemWidth(fieldWidth-(jobPram?160.0f:90.0f));
		std::string jobName = "createplot-"+std::to_string(JobCreatePlot::jobIdCounter);
		ImGui::InputText("##jobName",&jobName);
		ImGui::PopItemWidth();
		if (jobPram) {
			ImGui::SameLine();
			ImGui::PushItemWidth(50.0f);
			if (ImGui::Button("Add Job")) {
				JobManager::getInstance().addJob(std::make_shared<JobCreatePlot>(jobName, *jobPram));
				JobCreatePlot::jobIdCounter++;
			}
			ImGui::PopItemWidth();
		}		
	}

	JobCreatePlot::JobCreatePlot(std::string title, const JobCreatePlotParam& param)
        : Job(title), param(param)
    {
		this->startRule.param = param.startRuleParam;
		this->finishRule.param = param.finishRuleParam;
		this->jobEditor.setData(&this->param);
    }

    JobCreatePlotParam* JobCreatePlot::drawUI()
    {
		static WidgetCreatePlot createPlotWidget;
		static JobCreatePlotParam createPlotParam;
		createPlotWidget.setData(&createPlotParam);
		if (createPlotWidget.draw()) {
			return createPlotWidget.getData();
		}
		else {
			return nullptr;
		}
	}

	int JobCreatePlot::jobIdCounter = 1;

	bool JobCreatePlot::isRunning() const {
		return this->running;
	}

	bool JobCreatePlot::isFinished() const {
		return this->finished;
	}

	bool JobCreatePlot::isPaused() const {
		return this->paused;
	}

    bool JobCreatePlot::start() {
		this->running = true;
		this->paused = false;
		return true;
	}

	bool JobCreatePlot::pause() {
		this->paused = true;
		return true;
	}

	bool JobCreatePlot::cancel() {
		this->running = false;
		this->paused = false;
		return false;
	}

	float JobCreatePlot::getProgress() {
		this->progress += 0.01f;
		if (this->progress > 1.0f) {
			this->progress = 0.0f;
		}
		return this->progress;
	}

	gui::JobRule& JobCreatePlot::getStartRule() {
		return this->startRule;
	}

	gui::JobRule& JobCreatePlot::getFinishRule() {
		return this->finishRule;
	}

	void JobCreatePlot::drawItemWidget() {
		Job::drawItemWidget();		

		if (!this->isRunning()) {			
			if (this->startRule.drawItemWidget()) {
				ImGui::ScopedSeparator();
			}
		}
		this->finishRule.drawItemWidget();
	}

	void JobCreatePlot::drawStatusWidget() {
		Job::drawStatusWidget();
		ImGui::ScopedSeparator();
		if (!this->isRunning()) {
			this->jobEditor.draw();
		}		
	}

    WidgetCreatePlot::WidgetCreatePlot() {
	}

    bool WidgetCreatePlot::draw()
    {
		bool result = false;
		ImGui::PushID((const void*)this);
		float fieldWidth = ImGui::GetWindowContentRegionWidth();

		ImGui::PushItemWidth(80.0f);
		ImGui::Text("Pool Key");
		ImGui::PopItemWidth();
		ImGui::SameLine(80.0f);
		ImGui::PushItemWidth(fieldWidth-(80.0f + 55.0f));
		result |= ImGui::InputText("##poolkey", &this->param->poolKey,ImGuiInputTextFlags_CharsHexadecimal);
		if (ImGui::IsItemHovered() && !this->param->poolKey.empty()) {
			ImGui::BeginTooltip();
			tooltiipText(this->param->poolKey.c_str());
			ImGui::EndTooltip();
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(55.0f);
		if (ImGui::Button("Paste##pool")) {
			this->param->poolKey = ImGui::GetClipboardText();
			result |= true;
		}
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(80.0f);
		ImGui::Text("Farm Key");
		ImGui::PopItemWidth();
		ImGui::SameLine(80.0f);
		ImGui::PushItemWidth(fieldWidth-(80.0f + 55.0f));
		ImGui::InputText("##farmkey", &this->param->farmKey,ImGuiInputTextFlags_CharsHexadecimal);
		if (ImGui::IsItemHovered() && !this->param->farmKey.empty()) {
			ImGui::BeginTooltip();
			tooltiipText(this->param->farmKey.c_str());
			ImGui::EndTooltip();
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(55.0f);
		if (ImGui::Button("Paste##farm")) {
			this->param->farmKey = ImGui::GetClipboardText();
			result |= true;
		}
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(80.0f);
		ImGui::Text("Temp Dir");
		ImGui::PopItemWidth();
		ImGui::SameLine(80.0f);
		static std::string tempDir;
		ImGui::PushItemWidth(fieldWidth-(80.0f + 105.0f));
		if (ImGui::InputText("##tempDir", &tempDir)) {
			this->param->tempPath = tempDir;
			result |= true;
		}
		if (ImGui::IsItemHovered() && !tempDir.empty()) {
			ImGui::BeginTooltip();
			tooltiipText(tempDir.c_str());
			ImGui::EndTooltip();
		}
		ImGui::PopItemWidth();
		
		ImGui::PushItemWidth(55.0f);
		ImGui::SameLine();		
		if (ImGui::Button("Paste##tempDir")) {
			tempDir = ImGui::GetClipboardText();
			this->param->tempPath = tempDir;
			result |= true;
		}
		ImGui::PopItemWidth();
		ImGui::PushItemWidth(55.0f);
		ImGui::SameLine();
		if (ImGui::Button("Select##tempDir")) {
			std::optional<std::filesystem::path> dirPath = ImFrame::PickFolderDialog();
			if (dirPath) {
				tempDir = dirPath->string();
				this->param->tempPath = tempDir;
			}
			result |= true;
		}
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(80.0f);
		ImGui::Text("Dest Dir");
		ImGui::PopItemWidth();
		ImGui::SameLine(80.0f);
		static std::string destDir;
		ImGui::PushItemWidth(fieldWidth-(80.0f + 105.0f));
		if (ImGui::InputText("##destDir", &destDir)) {
			this->param->destPath = destDir;
			result |= true;
		}
		if (ImGui::IsItemHovered() && !destDir.empty()) {
			ImGui::BeginTooltip();
			tooltiipText(destDir.c_str());
			ImGui::EndTooltip();
		}
		ImGui::PopItemWidth();
		ImGui::PushItemWidth(55.0f);
		ImGui::SameLine();
		if (ImGui::Button("Paste##destDir")) {
			destDir = ImGui::GetClipboardText();
			this->param->destPath = destDir;
			result |= true;
		}
		ImGui::PopItemWidth();
		ImGui::PushItemWidth(55.0f);
		ImGui::SameLine();
		if (ImGui::Button("Select##destDir")) {
			std::optional<std::filesystem::path> dirPath = ImFrame::PickFolderDialog();
			if (dirPath) {
				destDir = dirPath.value().string();
				this->param->destPath = dirPath.value();
			}
			result |= true;
		}
		ImGui::PopItemWidth();

		if (ImGui::CollapsingHeader("Advanced")) {
			ImGui::Indent(20.0f);
			fieldWidth = ImGui::GetWindowContentRegionWidth();

			ImGui::Text("Plot Size (k)");
			ImGui::SameLine(120.0f);
			ImGui::PushItemWidth(fieldWidth-130.0f);
			if (ImGui::InputInt("##k", &this->param->ksize, 1, 10)) {
				if (this->param->ksize < 1) {
					this->param->ksize = 1;
				}
				result |= true;
			}
			ImGui::PopItemWidth();
			
			ImGui::Text("Temp Dir2");
			ImGui::SameLine(120.0f);
			static std::string tempDir2;
			ImGui::PushItemWidth(fieldWidth-225.0f);
			if (ImGui::InputText("##tempDir2", &tempDir2)) {
				this->param->temp2Path = tempDir2;
				result |= true;
			}
			if (ImGui::IsItemHovered() && !tempDir2.empty()) {
				ImGui::BeginTooltip();
				tooltiipText(tempDir2.c_str());
				ImGui::EndTooltip();
				result |= true;
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(50.0f);
			if (ImGui::Button("Paste##tempDir2")) {
				tempDir2 = ImGui::GetClipboardText();
				this->param->temp2Path = tempDir2;
				result |= true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Select##tempDir2")) {
				std::optional<std::filesystem::path> dirPath = ImFrame::PickFolderDialog();
				if (dirPath) {
					tempDir2 = dirPath.value().string();
					this->param->temp2Path = dirPath.value();
				}
				result |= true;
			}
			ImGui::PopItemWidth();

			ImGui::Text("Buckets");
			ImGui::SameLine(120.0f);
			ImGui::PushItemWidth(fieldWidth-130.0f);
			if (ImGui::InputInt("##buckets", &this->param->buckets, 1, 8)) {
				if (this->param->buckets < 16) {
					this->param->buckets = 16;
				}
				if (this->param->buckets > 128) {
					this->param->buckets = 128;
				}
				result |= true;
			}

			ImGui::Text("Stripes");
			ImGui::SameLine(120.0f);
			ImGui::PushItemWidth(fieldWidth-130.0f);
			if (ImGui::InputInt("##stripes", &this->param->stripes, 1024, 4096)) {
				if (this->param->stripes < 1) {
					this->param->stripes = 1;
				}
				result |= true;
			}
			ImGui::PopItemWidth();

			ImGui::Text("Threads");
			ImGui::SameLine(120.0f);
			ImGui::PushItemWidth(fieldWidth-130.0f);
			if (ImGui::InputInt("##threads", &this->param->threads, 1, 2)) {
				if (this->param->threads < 1) {
					this->param->threads = 1;
				}
				result |= true;
			}
			ImGui::PopItemWidth();

			ImGui::Text("Buffer (MB)");
			ImGui::SameLine(120.0f);
			ImGui::PushItemWidth(fieldWidth-130.0f);
			if (ImGui::InputInt("##buffer", &this->param->buffer, 1024, 2048)) {
				if (this->param->buffer < 16) {
					this->param->buffer = 16;
				}
				result |= true;
			}
			ImGui::PopItemWidth();

			ImGui::Text("Bitfield");
			ImGui::SameLine(120.0f);
			ImGui::PushItemWidth(fieldWidth-130.0f);
			ImGui::Checkbox("##bitfeld", &this->param->bitfied);
			ImGui::PopItemWidth();

			if (ImGui::Button("Load Default")) {
				this->param->loadDefault();
				result |= true;
			}

			ImGui::Unindent(20.0f);
		}

		if (ImGui::CollapsingHeader("Start Rule")) {
			ImGui::Indent(20.0f);
			result |= this->startRuleWidget.draw();
			ImGui::Unindent(20.0f);
		}

		if (ImGui::CollapsingHeader("Finish Rule")) {
			ImGui::Indent(20.0f);
			result |= this->finishRuleWidget.draw();
			ImGui::Unindent(20.0f);
		}

		std::string errMsg = "";
		result = this->param->isValid(&errMsg);

		if (!errMsg.empty()) {
			ImGui::Text(errMsg.c_str());
		}

		ImGui::PopID();
		return result;
	}

	void WidgetCreatePlot::setData(JobCreatePlotParam* param)
    {
        Widget<JobCreatePlotParam*>::setData(param);
        this->startRuleWidget.setData(&param->startRuleParam);
		this->finishRuleWidget.setData(&param->finishRuleParam);
    }

    void JobCreatePlotParam::loadDefault()
    {
		this->buckets = 128;
		this->stripes = 65536;
		this->threads = 2;
		this->buffer = 4608;
		this->ksize = 32;
		this->temp2Path = "";
		this->bitfied = true;
	}

	bool JobCreatePlotParam::isValid(std::string* errMsg) const {
		if (errMsg) {
			if (this->poolKey.empty()) {
				*errMsg = "pool public key must be specified";
			}
			else {
				if (this->poolKey.length() < 2*48) {
					*errMsg = "pool public key must be 48 bytes (96 hex characters)";
				}
			}
			if (this->farmKey.empty()) {
				*errMsg = "farm public key must be specified";
			}
			else {
			if (this->farmKey.length() < 2*48) {
					*errMsg = "farm public key must be 48 bytes (96 hex characters)";
				}
			}
		}
		return !this->poolKey.empty() && !this->farmKey.empty();
	}

    bool WidgetCreatePlotStartRule::draw()
    {
		ImGui::PushID((const void*)this);
		bool result = false;
		if (ImGui::Checkbox("Paused", &this->param->startPaused)) {
			this->param->startDelayed = false;
			this->param->startConditional = false;
			this->param->startImmediate = false;
			result |= true;
		}
		if (ImGui::Checkbox("Immediately", &this->param->startImmediate)) {
			this->param->startDelayed = false;
			this->param->startConditional = false;
			this->param->startPaused = false;
			result |= true;
		}
		if (ImGui::Checkbox("Delayed",&this->param->startDelayed)) {
			this->param->startImmediate = false;
			this->param->startConditional = false;
			this->param->startPaused = false;
			result |= true;
		}
		if (this->param->startDelayed) {
			ImGui::Indent(20.0f);
			float fieldWidth = ImGui::GetWindowContentRegionWidth();
			ImGui::BeginGroupPanel(ImVec2(-1.0f,0.0f));
			ImGui::Text("Delay (min)");
			ImGui::SameLine(80.0f);
			ImGui::PushItemWidth(fieldWidth-160.0f);
			if (ImGui::InputInt("##delay", &this->param->startDelayedMinute, 1, 5)) {
				if (this->param->startDelayedMinute < 1) {
					this->param->startDelayedMinute = 1;
				}
				result |= true;
			}
			ImGui::PopItemWidth();
			ImGui::EndGroupPanel();
			ImGui::Unindent(20.0f);
		}
		if (ImGui::Checkbox("Conditional",&this->param->startConditional)) {
			this->param->startImmediate = false;
			this->param->startDelayed = false;
			this->param->startPaused = false;
			result |= true;
		}
		if (this->param->startConditional) {
			ImGui::Indent(20.0f);
			ImGui::BeginGroupPanel(ImVec2(-1.0f,0.0f));
			
			result |= ImGui::Checkbox("if Active Jobs",&this->param->startCondActiveJob);
			if (this->param->startCondActiveJob) {
				float fieldWidth = ImGui::GetWindowContentRegionWidth();
				ImGui::Text("Less Than");
				ImGui::SameLine(80.0f);
				ImGui::PushItemWidth(fieldWidth-150.0f);
				if (ImGui::InputInt("##lessthan", &this->param->startCondActiveJobCount, 1, 5)) {
					if (this->param->startCondActiveJobCount < 1) {
						this->param->startCondActiveJobCount = 1;
					}
					result |= true;
				}
				ImGui::PopItemWidth();
			}
			result |= ImGui::Checkbox("If Time Within",&this->param->startCondTime);
			if (this->param->startCondTime) {
				float fieldWidth = ImGui::GetWindowContentRegionWidth();
				ImGui::Text("Start (Hr)");
				ImGui::SameLine(80.0f);
				ImGui::PushItemWidth(fieldWidth-150.0f);
				if (ImGui::InputInt("##start", &this->param->startCondTimeStart, 1, 5)) {
					if (this->param->startCondTimeStart < 0) {
						this->param->startCondTimeStart = 24;
					}
					if (this->param->startCondTimeStart > 24) {
						this->param->startCondTimeStart = 0;
					}
					result |= true;
				}
				ImGui::PopItemWidth();
				ImGui::Text("End (Hr)");
				ImGui::SameLine(80.0f);
				ImGui::PushItemWidth(fieldWidth-150.0f);
				if (ImGui::InputInt("##end", &this->param->startCondTimeEnd, 1, 5)) {
					if (this->param->startCondTimeEnd < 0) {
						this->param->startCondTimeEnd = 24;
					}
					if (this->param->startCondTimeEnd > 24) {
						this->param->startCondTimeEnd = 0;
					}
					result |= true;
				}
				ImGui::PopItemWidth();
			}
			
			ImGui::EndGroupPanel();
			ImGui::Unindent(20);
		}
		if (!this->param->startImmediate && !this->param->startDelayed && !this->param->startConditional && !this->param->startPaused) {
			this->param->startPaused = true;
		}
		ImGui::PopID();
		return result;
	}

	bool WidgetCreatePlotFinishRule::draw() {
		ImGui::PushID((const void*)this);
		bool result = false;
		result |= ImGui::Checkbox("Relaunch",&this->param->repeatJob);
		if (this->param->repeatJob) {
			ImGui::Indent(20.0f);
			ImGui::BeginGroupPanel(ImVec2(-1.0f,0.0f));
			if (!this->param->repeatIndefinite) {
				float fieldWidth = ImGui::GetWindowContentRegionWidth();
				ImGui::Text("Count");
				ImGui::SameLine(80.0f);
				ImGui::PushItemWidth(fieldWidth-150.0f);
				if (ImGui::InputInt("##repeatCount", &this->param->repeatCount, 1, 5)) {
					if (this->param->repeatCount < 1) {
						this->param->repeatCount = 1;
					}
					result |= true;
				}
				ImGui::PopItemWidth();
			}
			result |= ImGui::Checkbox("Indefinite",&this->param->repeatIndefinite);
			ImGui::EndGroupPanel();
			ImGui::Unindent(20.0f);
		}
		result |= ImGui::Checkbox("Launch Program",&this->param->execProg);
		if (this->param->execProg) {
			ImGui::Indent(20.0f);
			ImGui::BeginGroupPanel(ImVec2(-1.0f,0.0f));
				float fieldWidth = ImGui::GetWindowContentRegionWidth();
				ImGui::Text("Path");
				ImGui::SameLine(80.0f);
				static std::string execPath;
				ImGui::PushItemWidth(fieldWidth-240.0f);
				if (ImGui::InputText("##execPath", &execPath)) {
					this->param->progToExec = execPath;
					result |= true;
				}
				if (ImGui::IsItemHovered() && !execPath.empty()) {
					ImGui::BeginTooltip();
					tooltiipText(execPath.c_str());
					ImGui::EndTooltip();
				}
				ImGui::PopItemWidth();
				ImGui::SameLine();
				ImGui::PushItemWidth(44.0f);
				if (ImGui::Button("Paste##execPath")) {
					execPath = ImGui::GetClipboardText();
					this->param->progToExec = execPath;
					result |= true;
				}
				ImGui::SameLine();
				if (ImGui::Button("Select##execPath")) {
					std::vector<ImFrame::Filter> filters;
					filters.push_back(ImFrame::Filter());
					filters.push_back(ImFrame::Filter());
					filters[0].name = "exe";
					filters[0].spec = "*.exe";
					filters[1].name = "batch script";
					filters[1].spec = "*.bat";
					std::optional<std::filesystem::path> dirPath = ImFrame::OpenFileDialog(filters);
					if (dirPath) {
						execPath = dirPath->string();
						this->param->progToExec = dirPath.value();
					}
					result |= true;
				}
				ImGui::PopItemWidth();
			ImGui::EndGroupPanel();
			ImGui::Unindent(20.0f);
		}
		ImGui::PopID();
		return result;
	}

	std::string Job::getTitle() const { return this->title; }

	void Job::drawItemWidget() {
		ImGui::Text(this->getTitle().c_str());
		ImGui::ProgressBar(this->getProgress());
	}

	void Job::drawStatusWidget() {
		ImGui::Text(this->getTitle().c_str());
		ImGui::ProgressBar(this->getProgress());
		if (this->isRunning()) {
			if(this->isPaused()){
				if (ImGui::Button("Resume")) {
					this->start();
				}
			}
			else {
				if (ImGui::Button("Pause")) {
					this->pause();
				}
			}
		}
		else {
			if (this->getStartRule().evaluate()) {
				if (ImGui::Button("Start")) {
					this->start();
				}
			}
			else {
				if (ImGui::Button("Overide Start")) {
					this->start();
				}
			}
			
		}
		ImGui::SameLine();
		if (this->isRunning()) {
			if (ImGui::Button("Cancel")) {
				this->cancel();
			}
		}
		else {
			if (ImGui::Button("Delete")) {
				this->cancel();
			}
		}
	}

    JobCratePlotStartRule::JobCratePlotStartRule()
    {
		this->creationTime = std::chrono::system_clock::now();
	}

    bool JobCratePlotStartRule::drawItemWidget()
    {
		if (this->param.startDelayed) {
			std::chrono::minutes delayDuration(this->param.startDelayedMinute);
			std::chrono::time_point<std::chrono::system_clock> startTime = this->creationTime + delayDuration;
			std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();
			std::chrono::duration<float> delta = startTime - currentTime;
			ImGui::Text("will start in %.1f hour ", delta.count()/3600.0f);
			return true;
		}
		else if (this->param.startPaused) {
			ImGui::Text("job paused");
			return true;
		}
		else if (this->param.startConditional) {
			bool result = false;
			if (this->param.startCondActiveJob) {
				ImGui::Text("start if active job < %d", this->param.startCondActiveJobCount);
				result |= true;
			}
			if (this->param.startCondTime) {
				ImGui::Text("start within %02d:00 - %2d:00", this->param.startCondTimeStart, this->param.startCondTimeEnd);
				result |= true;
			}
			return result;
		}
		return false;
	}

	bool JobCratePlotStartRule::evaluate() {
		if (this->param.startDelayed) {
			std::chrono::minutes delayDuration(this->param.startDelayedMinute);
			std::chrono::time_point<std::chrono::system_clock> startTime = this->creationTime + delayDuration;
			std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();
			return currentTime > startTime;
		}
		else if (this->param.startConditional) {
			bool result = true;
			if (this->param.startCondTime) {
				std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();
				std::time_t ct = std::chrono::system_clock::to_time_t(currentTime);
				std::tm* t = std::localtime(&ct);
				if ((this->param.startCondTimeStart > t->tm_hour) && 
					(t->tm_hour < this->param.startCondTimeEnd)) {
					result &= true;
				}
				else {
					result &= false;
				}
			}

			if (this->param.startCondActiveJob) {
				if (JobManager::getInstance().countRunningJob() < this->param.startCondActiveJobCount) {
					result &= true;
				}
				else {
					result &= false;
				}
			}
			return result;
		}
	}

    JobCratePlotStartRuleParam::JobCratePlotStartRuleParam() {
		
	}

	bool JobCreatePlotFinishRule::drawItemWidget() {
		if (this->param.repeatJob) {
			if (this->param.repeatIndefinite) {
				ImGui::Text("Job will repeat indefinitely");
				return true;
			}
			else {
				ImGui::Text("Job will repeat %d times", this->param.repeatCount);
				return true;
			}
		}
		return false;
	}

	JobManager& JobManager::getInstance() {
		static JobManager instance;
		return instance;
	}

	void JobManager::addJob(std::shared_ptr<Job> newJob) {
		this->activeJobs.push_back(newJob);
	}

	void JobManager::setSelectedJob(std::shared_ptr<Job> job) {
		this->selectedJob = job;
	}

	std::shared_ptr<gui::Job> JobManager::getSelectedJob() const {
		return this->selectedJob;
	}

	size_t JobManager::countJob() const {
		return this->activeJobs.size();
	}

	size_t JobManager::countRunningJob() const {
		size_t result = 0;
		for (auto job : this->activeJobs) {
			if (job->isRunning()) {
				result++;
			}
		}
		return result;
	}

	std::vector<std::shared_ptr<Job>>::const_iterator JobManager::jobIteratorBegin() const  {
		return this->activeJobs.begin();
	}

	std::vector<std::shared_ptr<Job>>::const_iterator JobManager::jobIteratorEnd() const  {
		return this->activeJobs.end();
	}

}  // namespace gui
