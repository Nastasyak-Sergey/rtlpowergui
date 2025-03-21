#pragma once
#include "RTLPowerWrapper.h"
//#include <map>
#include <regex>
#include <fstream>
#include <unordered_map>
#include <optional>

struct Settings
{
	int samp_rate;
	float min_freq;
	float max_freq;
	// 0 = Hz
	// 1 = kHz
	// 2 = MHz
	// 3 = GHz
	int min_freq_units;
	float gain;
	int max_freq_units;
	int nbins;
	int percent;
	int nsamples;
	void to_stringstream(std::stringstream& s);
	static Settings from_stringstream(std::stringstream& str);

	// Practical equality check for baseline
	bool operator==(const Settings& b) const;
};

struct Scan
{
	std::vector<Readout> reads;
	bool is_first_of_scan;
	bool is_last_of_scan;
};


struct Measurement
{
	Settings settings;
	std::vector<double> spectrum;
	std::vector<double> average;
	std::vector<double> max;
	std::vector<double> min;
	int numScans = 0;
	int stepFreq = 0;

	double get_bin_center_freq(size_t idx);
	size_t get_bin_for_freq(double freq);
	double get_low_freq();
	double get_high_freq();
	double get_freq_range() { return get_high_freq() - get_low_freq(); }
	double get_hertz_per_bin();
	double get_bin_scale();
	int64_t get_freq(float val, int units);
	size_t get_number_of_scans();

	std::string to_csv();
	static Measurement from_csv(const std::string& str);

	static void from_binFile_meta(const std::string& fname, Measurement& bin);
	static void from_binFile_raw(const std::string& fname, Measurement& raw);

	std::vector<double>& get_baseline_bin(int baseline_mode);
};

struct Measure
{
	bool show_spectrum;
	bool show_average;
	bool show_max;
	bool show_min;
	Measurement meas;
};

// Runs in a thread to build the raw plots for ImGui
// with no blocking
// and uses the RTLPowerWrapper to get its data
// Basically the whole application except GUI is here
class PlotBuilder
{
private:
	RTLPowerWrapper power_wrapper;
	std::thread thread;
	bool thread_run;
	// We neatly subdivide the freq spectrum, and round samples
	// (this will nearly never be an issue)

	std::mutex mtx;
	std::vector<Scan> reads_buffer;
	std::atomic<bool> next_is_first;

	double bandwidths[4] =
			{
			100e3,
			1e6,
			2e6
			};

public:

	int baseline_mode;

	void update();
	std::atomic<bool> launch_queued = false;

	Settings exposed;

	void commit_settings();
	void update_averaging();

	bool can_change_settings();

	void launch();
	// Updates dynamic graphs
	//void update(float dt);
	
	void stop();

	// Returns Hertz / dB/Hz
	Measurement current;
	int num_average_hold;
	// Number of measurements since last update_averaging
	// growing until it's equal to num_average_hold
	int measurement_count;
	std::vector<std::vector<double>> prev_measurements;

	std::vector<Measurement> measures;
	// Settings must match current, otherwise it's ignored
	std::optional<Measurement> baseline;

	bool has_baseline();

	bool get_power_status() { return power_wrapper.get_exec_status(); }

	PlotBuilder();
	~PlotBuilder();

};
