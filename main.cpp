#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <vector>
namespace {
	using std::vector;
	using std::string;
	using std::runtime_error;
	using std::cout;
	using std::cerr;
	using std::endl;
	using std::ios_base;
	namespace fs = std::filesystem;

	struct Error_Reporter {
		int exit_code = EXIT_SUCCESS;
		const char* const program_name;

		Error_Reporter(const char* const pn)
		  : program_name{ pn }
		{
			cerr.exceptions(ios_base::badbit | ios_base::failbit | ios_base::eofbit);
		}

		/**
		 * Present to the user a report of an exception within the program.
		 */
		void report_exception(const char* const title, const string& what = {}) noexcept
		{
			try {
				exit_code = EXIT_FAILURE;
				const char* const prefix_tag = "!!! ";
				cerr << prefix_tag << program_name << endl;
				cerr << prefix_tag << title << " error";
				if (what.empty()) {
					cerr << '.' << endl;
				}
				else {
					cerr << ':' << endl;
					cerr << prefix_tag << what << endl;
				}
			}
			catch (...) {
				std::terminate();
			}
		}
	};


	class Program {
		const int argc;
		char** const argv;
		const vector<int> args_idx;
		const vector<int> flags_idx;
		const fs::path dst_dir;
		const bool bVerbose;
		const fs::copy_options copy_options = fs::copy_options::skip_existing;


		vector<int> find_args()
		{
			vector<int> args;
			for (int idx = 1; idx < argc; ++idx)
				if (*argv[idx] != '-')
					args.push_back(idx);
			return args;
		}


		vector<int> find_flags()
		{
			vector<int> flags;
			for (int idx = 1; idx < argc; ++idx)
				if (*argv[idx] == '-')
					flags.push_back(idx);
			return flags;
		}


		bool exists_flag(const char* const flag)
		{
			for (int idx : flags_idx)
				if (std::strcmp(argv[idx], flag) == 0)
					return true;
			return false;
		}


		void verbose_message(string const& msg)
		{
			if (bVerbose) {
				const fs::path program_path{ argv[0] };
				cout << '[' << program_path.filename() << ']' << ' ' << msg << endl;
			}
		}

	public:
		Program(const int argc, char** argv)
		  : argc{ argc }
		  , argv{ argv }
		  , args_idx{ find_args() }
		  , flags_idx{ find_flags() }
		  , dst_dir{ fs::canonical(argv[args_idx.front()]) }
		  , bVerbose{ exists_flag("-v") }
		{
			assert(1 < argc);
			if (!fs::is_directory(dst_dir))
				throw runtime_error{ "No such directory: " + dst_dir.string() };
			verbose_message("Destination: " + dst_dir.string());
		}

		void run()
		{
			if (args_idx.empty())
				return;
			auto idx_itr = args_idx.begin();
			auto idx_end = args_idx.end();
			while (++idx_itr != idx_end) { // Skip the first which is the dst_dir.
				const int idx = *idx_itr;
				fs::path lib_path{ argv[idx] };
				if (lib_path.extension() != ".lib")
					throw runtime_error{ "Bad lib file: " + lib_path.string() };
				lib_path.replace_extension(".dll");
				if (!fs::exists(lib_path))
					throw runtime_error{ "No such DLL: " + lib_path.string() };
				fs::copy(lib_path, dst_dir, copy_options);
				verbose_message("Copied: " + lib_path.string());
			}
		}
	};
} // namespace

int main(const int argc, char** argv)
{
	Error_Reporter e{ argv[0] };
	try {
		if (1 < argc) {
			Program copy_install_dlls{ argc, argv };
			copy_install_dlls.run();
		}
	}
	catch (std::bad_alloc const&) {
		e.report_exception("Out of memory");
	}
	catch (std::filesystem::filesystem_error const& ex) {
		e.report_exception("File system", ex.what());
	}
	catch (std::ios_base::failure const& ex) {
		e.report_exception("I/O", ex.what());
	}
	catch (std::invalid_argument const& ex) {
		e.report_exception("Invalid argument", ex.what());
	}
	catch (std::runtime_error const& ex) {
		e.report_exception("Runtime", ex.what());
	}
	catch (std::exception const& ex) {
		e.report_exception("Exception", ex.what());
	}
	catch (...) {
		e.report_exception("Unexpected");
	}
	return e.exit_code;
}
