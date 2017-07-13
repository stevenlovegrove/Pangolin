#include <pangolin/pangolin.h>
#include <pangolin/utils/argagg.hpp>
#include <pangolin/utils/file_utils.h>

#include <functional>
#include <thread>

#include "csv_data_loader.h"

template<>
pangolin::Rangef argagg::convert::arg<pangolin::Rangef>(char const* str)
{
    std::stringstream ss(str);
    pangolin::Rangef r;
    ss >> r.min; ss.get(); ss >> r.max;
    return r;
}

int main( int argc, char* argv[] )
{
    // Parse command line
    argagg::parser argparser {{
        { "help", {"-h", "--help"}, "Print usage information and exit.", 0},
        { "header", {"-H","--header"}, "Treat 1st row as column titles", 0},
        { "x", {"-x"}, "X-axis variable (default: '$i')", 1},
        { "y", {"-y"}, "Y-axis series to plot, seperated by commas (eg: '$0,sin($1),sqrt($2+$3)' )", 1},
        { "delim", {"-d"}, "expected column delimitter (default: ',')", 1},
        { "xrange", {"-X","--x-range"}, "X-Axis min:max view (default: '0:100')", 1},
        { "yrange", {"-Y","--y-range"}, "Y-Axis min:max view (default: '0:100')", 1},
    }};

    argagg::parser_results args = argparser.parse(argc, argv);
    if ( (bool)args["help"] || !args.pos.size()) {
        std::cerr << "Usage: Plotter [options] file1.csv [fileN.csv]*" << std::endl
                  << argparser << std::endl
                  << "    where: $i is a placeholder for the datum index," << std::endl
                  << "           $0, $1, ... are placeholders for the 0th, 1st, ... sequential datum values over the input files" << std::endl;
        return 0;
    }

    // Default values
    const std::string x = args["x"].as<std::string>("$i");
    const std::string ys = args["y"].as<std::string>("$0");
    const char delim = args["delim"].as<char>(',');
    const pangolin::Rangef xrange = args["xrange"].as<>(pangolin::Rangef(0.0f,100.0f));
    const pangolin::Rangef yrange = args["yrange"].as<>(pangolin::Rangef(0.0f,100.0f));

    pangolin::DataLog log;

    CsvDataLoader csv_loader(args.all_as<std::string>(), delim);

    if(args["header"]) {
        std::vector<std::string> labels;
        csv_loader.ReadRow(labels);
        log.SetLabels(labels);
    }

    // Load asynchronously incase the file is large or is being read interactively from stdin
    bool keep_loading = true;
    std::thread data_thread([&](){
        std::vector<std::string> row;

        while(keep_loading && csv_loader.ReadRow(row)) {
            std::vector<float> row_num(row.size(), std::numeric_limits<float>::quiet_NaN() );
            for(size_t i=0; i< row_num.size(); ++i) {
                try{
                    row_num[i] = std::stof(row[i]);
                }catch(const std::invalid_argument& e){
                    std::cerr << "Warning: couldn't parse '" << row[i] << "' as numeric data (use -H option to include header)" << std::endl;
                }
            }
            log.Log(row_num);
        }
    });

    pangolin::CreateWindowAndBind("Plotter", 640, 480);

    pangolin::Plotter plotter(&log, xrange.min, xrange.max, yrange.min, yrange.max, 0.001, 0.001);
    if( (bool)args["x"] || (bool)args["y"]) {
        plotter.ClearSeries();
        std::vector<std::string> yvec = pangolin::Split(ys,',');
        for(const std::string& y : yvec) {
            plotter.AddSeries(x,y);
        }
    }

    plotter.SetBounds(0.0, 1.0, 0.0, 1.0);
    pangolin::DisplayBase().AddDisplay(plotter);

    while( !pangolin::ShouldQuit() )
    {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      pangolin::FinishFrame();
    }

    keep_loading = false;
    data_thread.join();

    return 0;
}
