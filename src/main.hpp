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
#include <chrono>

namespace gui
{
	class UiWidget {
	public:
		UiWidget(){};
		virtual bool draw() = 0;
	};

	template<typename P> class Widget : UiWidget {
	public:
		Widget():UiWidget(){};
		virtual void setData(P param) {
			this->param = param;
		}
		P getData() {
			return param;
		}
		const P getData() const {
			return param;
		}
	protected:
		P param;
	};

	class JobRule {
	public:
		JobRule(){};
		virtual bool evaluate(){ return true; };
		virtual bool drawItemWidget() { return false; };
	};

	class Job {
	public:
		Job(std::string title):title(title){};
		virtual bool isRunning() const = 0;
		virtual bool isPaused() const = 0;
		virtual bool isFinished() const = 0;
		virtual bool start() = 0;
		virtual bool pause() = 0;
		virtual bool cancel() = 0;
		virtual float getProgress() = 0;
		virtual JobRule& getStartRule() = 0;
		virtual JobRule& getFinishRule() = 0;
		virtual ~Job(){};
		std::string getTitle() const;
		virtual void drawItemWidget();
		virtual void drawStatusWidget();
	protected:
		std::string title;
	};

	class JobStartRule : public JobRule {
	public:
	};

	class JobFinishRule : public JobRule {
	public:
	};

	class JobCratePlotStartRuleParam {
	public:
		JobCratePlotStartRuleParam();
		bool startImmediate {true};
		bool startDelayed {false};
		bool startPaused {false};
		bool startConditional {false};
		bool startCondActiveJob {true};
		bool startCondTime {false};
		int startDelayedMinute {15};
		int startCondTimeStart {1};
		int startCondTimeEnd {6};
		int startCondActiveJobCount {1};
	};

	class JobCratePlotStartRule : public JobStartRule {
	public:
		JobCratePlotStartRule();
		bool drawItemWidget() override;
		bool evaluate() override;
		JobCratePlotStartRuleParam param;
		std::chrono::time_point<std::chrono::system_clock> creationTime;
	};

	class JobCreatePlotFinishRuleParam {
	public:
		JobCreatePlotFinishRuleParam(){};
		bool repeatJob {true};
		bool repeatIndefinite {false};
		bool execProg {false};
		std::filesystem::path progToExec;
		std::filesystem::path progWorkingDir;
		bool execProgOnRepeat {true};
		int repeatCount {1};
	};

	class JobCreatePlotFinishRule : public JobFinishRule {
	public:
		bool drawItemWidget() override;
		JobCreatePlotFinishRuleParam param;
	};

	class JobCreatePlotParam {
	public:
		JobCreatePlotParam(){};
		std::filesystem::path destPath;
		std::filesystem::path tempPath;
		std::filesystem::path temp2Path;
		std::string poolKey;
		std::string farmKey;
		int ksize {32};
		int buckets {128};
		int stripes {65536};
		int threads {2};
		int buffer {4608};
		bool bitfied {true};
		JobCratePlotStartRuleParam startRuleParam;
		JobCreatePlotFinishRuleParam finishRuleParam;
		void loadDefault();
		bool isValid(std::string* errMsg = nullptr) const;
	};

	class WidgetCreatePlotStartRule : public Widget<JobCratePlotStartRuleParam*> {
	public:
		bool draw() override;
	};

	class WidgetCreatePlotFinishRule : public Widget<JobCreatePlotFinishRuleParam*> {
	public:
		bool draw() override;
	};

	class WidgetCreatePlot : public Widget<JobCreatePlotParam*> {
	public:
		WidgetCreatePlot();;
		bool draw() override;
		virtual void setData(JobCreatePlotParam* param) override;
	protected:
		WidgetCreatePlotStartRule startRuleWidget;
		WidgetCreatePlotFinishRule finishRuleWidget;
	};

	class JobCreatePlot : public Job {
	public:
		JobCreatePlot(std::string title) : Job(title){};
		JobCreatePlot(std::string title,const JobCreatePlotParam& param);
		static JobCreatePlotParam* drawUI();
		static int jobIdCounter;

		bool isRunning() const override;
		bool isFinished() const override;
		bool isPaused() const override;
		bool start() override;
		bool pause() override;
		bool cancel() override;
		float getProgress() override;
		JobRule& getStartRule() override;
		JobRule& getFinishRule() override;
		void drawItemWidget() override;
		void drawStatusWidget() override;
	protected:
		bool paused {false};
		bool running {false};
		bool finished {false};
		float progress {0.0f};
		JobCreatePlotParam param;
		JobCratePlotStartRule startRule;
		JobCreatePlotFinishRule finishRule;
		WidgetCreatePlot jobEditor;
	};

	class JobManager {
	public: 
		JobManager(){};
		static JobManager& getInstance();
		void addJob(std::shared_ptr<Job> newJob);
		void setSelectedJob(std::shared_ptr<Job> job);
		std::shared_ptr<Job> getSelectedJob() const;
		size_t countJob() const;
		size_t countRunningJob() const;
		std::vector<std::shared_ptr<Job>>::const_iterator jobIteratorBegin() const ;
		std::vector<std::shared_ptr<Job>>::const_iterator jobIteratorEnd() const ;
	protected:
		std::vector<std::shared_ptr<Job>> activeJobs;
		std::shared_ptr<Job> selectedJob {nullptr};
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