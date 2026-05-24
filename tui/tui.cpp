#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "lz77.h"
#include "HuffmanEncoding.h"
#include "lzw.h"

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <iomanip>
#include <sstream>
#include <thread>
#include <atomic>


using namespace ftxui;

// 1. Declare the vectors globally so the UI can access them anytime
std::vector<std::vector<int>> global_times_data;
std::vector<std::vector<double>> global_ratios_data;

bool loadComparisonsFromFile() {
    std::ifstream inFile("comparison_data.txt");
    if (!inFile) {
        return false;
    }

    global_ratios_data.clear();
    global_times_data.clear();

    for (int i = 0; i < 4; ++i) {
        double r1, r2, r3;
        if (inFile >> r1 >> r2 >> r3) {
            global_ratios_data.push_back({r1, r2, r3});
        }
    }

    for (int i = 0; i < 4; ++i) {
        int t1, t2, t3;
        if (inFile >> t1 >> t2 >> t3) {
            global_times_data.push_back({t1, t2, t3});
        }
    }

    inFile.close();
    return (global_ratios_data.size() == 4 && global_times_data.size() == 4);
}



std::string checkDecompressionAlgo(std::string filename) {
    std::string algo = "";
    size_t dash = filename.find_last_of("-");
        if (dash != std::string::npos) {
            algo = filename.substr(dash+1,4);
        }
    return algo;
}



// Place this right above your main() function
ftxui::Element RenderComparisonGraphs(const std::vector<std::vector<double>>& ratios, const std::vector<std::vector<int>>& times) {
 
    
    // Find maximums so the bars scale correctly (0.0 to 1.0)
    double max_ratio = 0.001;
    for (const auto& group : ratios) {
        for (double v : group) if (v > max_ratio) max_ratio = v;
    }
    
    int max_time = 1;
    for (const auto& group : times) {
        for (int v : group) if (v > max_time) max_time = v;
    }

    // Categories matching your compare functions and image
    std::vector<std::string> labels = {"Code File", "Log File", "Text File", "Spreadsheet"};
    std::vector<Color> colors = {Color::Blue, Color::Green, Color::Red}; // Huffman, LZW, LZ77
    
    // Helper lambda to generate a single grouped bar chart element
    auto make_chart = [&](const std::string& title, auto data, double max_val, bool is_float) {
        Elements groups;
        for (size_t i = 0; i < 4; ++i) { // 4 file categories
            Elements bars;
            for (size_t j = 0; j < 3; ++j) { // 3 algorithms
                double val = (double)data[i][j];
                float bar_height = static_cast<float>(val / max_val);
                
                // Format the number to print above/below the bar
                std::ostringstream out;
                if (is_float) out << std::fixed << std::setprecision(1) << val;
                else out << (int)val;
                
                bars.push_back(
                    vbox({
                        gaugeUp(bar_height) | color(colors[j]) | flex,
                        text(out.str()) | hcenter | dim
                    }) | flex
                );
                if (j < 2) bars.push_back(text(" ")); // Spacer between bars
            }
            
            groups.push_back(
                vbox({
                    hbox(bars) | flex,
                    separator(),
                    text(labels[i]) | hcenter
                }) | flex
            );
            if (i < 3) groups.push_back(separatorLight()); // Spacer between file groups
        }
        
        return vbox({
            text(title) | bold | hcenter,
            separator(),
            hbox(groups) | size(HEIGHT, GREATER_THAN, 12) // Ensure the graph has enough vertical space
        }) | border;
    };

    // Return the combined visual elements: Ratios Graph, Times Graph, and a Legend
    return vbox({
        make_chart("Compression Ratios", ratios, max_ratio, true),
        make_chart("Compression Times (ms)", times, (double)max_time, false),
        hbox({
            text("Legend: ") | bold,
            text("■ Huffman ") | color(colors[0]),
            text("■ LZW ") | color(colors[1]),
            text("■ LZ77 ") | color(colors[2])
        }) | hcenter
    });
}


int main(){
   
    

    std::string filename = "";
    std::string algo_message = "";
    std::string stats = "";
    std::string status_message = "";
    std::string file_compressed = "";
    std::string file_decompressed = "";
    
    int mainContainerFocus = 1;
    
    bool showAlgoButtons = false;
    bool fileCompressed = false;
    bool fileDecompressed = false;
    bool showStats = false;
    bool comparison = false;
    bool is_comparing = false; // Tracks if the background thread is running

    

    int compressionTime = 0 , compressionSize = 0 , compressionPercentage = 0;
    
    auto fileinput = Input(&filename, "Type Something...");

    auto screen = ScreenInteractive::Fullscreen();

    auto header = Container::Vertical({
            Renderer([] {
                return vbox({
                    text("File Compression Algorithms") | bold | center,
                    separator(),
                    text("This is a simple File compression Program which offers 3 distinct compression algos , Huffman , LZ77 and LZW.") | center,
                    separator()
                });
            }),
    });


    auto fileInputBar = Container::Vertical({
        fileinput,
    });

    auto fileInputRenderer = Renderer(fileInputBar, [&] {
        return vbox({
            text("Enter Filename: "),
            fileinput->Render() | border
        });
    });

    auto huffmanBtn = Button("Huffman Encoding", [&]{
        algo_message = "Compressing through Huffman Encoding ...";
        Huffman::huffmanFileCompress(filename);
        fileCompressed = true;
        file_compressed = "Your file is now compressed.";
        stats = Huffman::stats;

    });

    auto lz77Btn = Button("LZ77", [&]{
        algo_message = "Compressing through LZ77 Encoding ...";
        compressToBinaryFile(filename);
        fileCompressed = true;
        file_compressed = "Your file is now compressed.";
        stats = getCompressionStats(filename);
       
    });

    auto lzwBtn = Button("LZW", [&]{
        algo_message = "Compressing through LZW Encoding ...";
        LZW::compress(filename);
        fileCompressed = true;
        file_compressed = "Your file is now compressed.";
        stats= LZW::stats;
    });

    auto algoBtnContainer = Container::Horizontal({
        huffmanBtn,
        lz77Btn,
        lzwBtn,
    });

    auto algoBtnRenderer = Renderer(algoBtnContainer, [&] {
        return hbox ({
            huffmanBtn->Render(),
            text("                "),
            lz77Btn->Render(),
            text("                "),
            lzwBtn->Render()
        });
    });

    auto compressButton = Button("Compress", [&] {
        if(filename.empty()) {
            status_message = "Error: Please enter a filename first";
            showAlgoButtons = false;
        } else {
            
            status_message = "";
            showAlgoButtons = true;

        }
        screen.PostEvent(Event::Custom);
    });

    auto decompressButton = Button("Decompress", [&] {
        showAlgoButtons = false;
        showStats = false;
        fileCompressed = false;
         if(filename.empty()) {
            status_message = "Error: Please enter a filename first";   
        } else {
            status_message = "Decompressing the file.";
            std::string algo = checkDecompressionAlgo(filename);
            if (algo == "huff"){
                Huffman::huffmanFileDecompress(filename);
            } else if(algo == "lzwa"){
                LZW::decompress(filename);
            } else if(algo == "lz77"){
                decompressToTextFile(filename);
            } else {
                status_message = "Cannot decompress this file.";
            }
            fileDecompressed = true;
            file_decompressed = "Your file is decompressed.";
        }
        screen.PostEvent(Event::Custom);
    });

   auto compareBtn = Button("Compare" , [&] {
        if (loadComparisonsFromFile()) {
            comparison = true;
            status_message = "Comparison data loaded,";
        } else {
            comparison = false;
            status_message = "Error: comparison_data.txt not found. Run generator first.";
        }
        screen.PostEvent(Event::Custom);
    });

    auto statsBtn = Button("Show Stats" , [&] {
            showStats = true;
            screen.PostEvent(Event::Custom);
    });

    auto buttonContainer = Container::Horizontal({
        compressButton,
        decompressButton,
        compareBtn,
    });

    auto buttonRenderer = Renderer(buttonContainer, [&] {
        return hbox ({
            compressButton->Render(),
            text("                              "),
            decompressButton->Render(),
            text("                              "),
            compareBtn->Render(),
        });
    });



    auto mainContainer = Container::Vertical({
    fileInputBar,      // 1st Visually
    buttonContainer,   // 2nd Visually
    algoBtnContainer,  // 3rd Visually
    statsBtn,          // 4th Visually
}, &mainContainerFocus);

   auto mainRenderer = Renderer(mainContainer, [&] {
        return vbox({
            header->Render(),
            fileInputRenderer->Render(),
            buttonRenderer->Render(),
            text(status_message),
            fileDecompressed ? vbox({
                separator(),
                text(file_decompressed)
            }) : vbox() ,
            showAlgoButtons ? vbox({
                separator(),
                text("Choose Algorithm:") | bold,
                algoBtnRenderer->Render(),
                text(algo_message) | dim,
            }) : vbox(),
            fileCompressed ? vbox ({
                separator(),
                text(file_compressed),
                statsBtn->Render() | size(WIDTH, EQUAL, 20),
            }) : vbox(),
            showStats ? vbox ({
                separator(),
                text(stats),
                separator(),
            }) : vbox(),
            comparison ? vbox({
                separator(),
                RenderComparisonGraphs(global_ratios_data, global_times_data)
            }) : vbox(),
        });
   });

   // auto tabToggle = Toggle()



    
   

    screen.Loop(mainRenderer);
    

    //screen.Print();
    return 0;
}

