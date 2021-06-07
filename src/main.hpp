#include <ImFrame.h>
extern "C" {
#include <GLFW/glfw3.h>
}

#ifdef IMFRAME_WINDOWS
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <shellapi.h>
#endif

#include <string>

namespace gui
{
	class Job {
	public:
		Job(std::string title):title(title){};
		virtual bool isRunning() const = 0;
		virtual bool isFinished() const = 0;
		virtual bool start() = 0;
		virtual bool pause() = 0;
		virtual bool cancel() = 0;
		virtual float getProgress() const = 0;
		virtual ~Job(){};
		std::string getTitle() const {
			return this->title;
		}
	protected:
		std::string title;
	};

	class JobCreatePlot : Job {

	};

	class MainApp : public ImFrame::ImApp
	{
	public:
		MainApp(GLFWwindow * window);
		virtual ~MainApp() {}
		void OnUpdate() override;
	protected:
		int activeTab {0};
		void toolPage();
		void statPage();
		void systemPage();
		void helpPage();
		void createPlotDialog();
		int wx,wy,ww,wh;
	};
}