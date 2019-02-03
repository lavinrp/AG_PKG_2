#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/filebox.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/wvl.hpp>

#include "Poco/Exception.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"
#include "Poco/URIStreamOpener.h"
#include <Poco/Delegate.h>
#include <Poco/Net/ConsoleCertificateHandler.h>
#include <Poco/Net/FTPStreamFactory.h>
#include <Poco/Net/HTTPSStreamFactory.h>
#include <Poco/Net/KeyConsoleHandler.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Path.h>
#include <Poco/SharedPtr.h>
#include <Poco/Zip/Decompress.h>
#include <Poco/SharedPtr.h>

#include <boost/filesystem.hpp>

#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>


class SSLInitializer
{
public:
	SSLInitializer()
	{
		Poco::Net::initializeSSL();
	}
	
	~SSLInitializer()
	{
		Poco::Net::uninitializeSSL();
	}
};

void unzip_file_to_dest(Poco::Path path_to_zipped_folder, const std::string& destination)
{
    std::cout << "Unzipping..." << std::endl;
    std::ifstream inp(path_to_zipped_folder.toString().c_str(), std::ios::binary);
    Poco::Zip::Decompress dec(inp, Poco::Path(destination, Poco::Path::Style::PATH_GUESS));
    dec.decompressAllFiles();
    std::cout << "Done" << std::endl;
}


void download_file(const std::string& uri_to_download, const std::string& install_path)
{
    std::string content {};
    std::ofstream out_file(install_path, std::ofstream::binary);
    if (out_file.good())
    {
        try
        {
            Poco::URI uri(uri_to_download);
            std::unique_ptr<std::istream> pStr(Poco::URIStreamOpener::defaultOpener().open(uri));
            Poco::StreamCopier::copyStream(*pStr.get(), out_file);
            // for easy debug
            // Poco::StreamCopier::copyToString(*(pStr.get()), content);
            // std::cout << content << std::endl;
            // out_file << content;
        }
        catch (Poco::Exception& exc)
        {
            std::cerr << exc.displayText() << std::endl;
        }
    }
    else
    {
        std::cerr << "bad file path" << std::endl;
    }
}

void create_vsc_proj_in_folder(std::string project_dir, std::string install_dir)
{

    // ensure project dir exists
    boost::filesystem::path vscode_dir(project_dir);
    vscode_dir /= ".vscode";
    boost::filesystem::create_directories(vscode_dir);

    // settings path
    boost::filesystem::path settings_filepath = vscode_dir / "settings.json";
    
    // dependency locations
    boost::filesystem::path base_dependency_dir = boost::filesystem::path(install_dir) / "AG_Dependencies";
    // boost
    boost::filesystem::path boost_root = base_dependency_dir / "boost_1_69_0_libstdcpp_clang_7";
    // sfml
    boost::filesystem::path sfml_root = base_dependency_dir / "SFML-2-5-1_clang_7_0__libstdcpp";
    boost::filesystem::path sfml_lib_dir = sfml_root / "lib";
    boost::filesystem::path sfml_cmake_dir = sfml_lib_dir / "cmake" / "SFML";
    // tgui
    boost::filesystem::path tgui_root = base_dependency_dir / "TGUI-0.8.3_clang_7_libstdcpp";
    boost::filesystem::path tgui_lib_dir = tgui_root / "lib";
    boost::filesystem::path tgui_cmake_dir = tgui_lib_dir / "lib" / "cmake" / "TGUI";
    // box
    boost::filesystem::path box2d_root = base_dependency_dir / "Box2D-Jan-27-2019__clang__libstdcpp";

    // create json for settings
    auto vs_code_settings = R"(
        {   
            "cmake.configureSettings" : {
                "BOOST_ROOT" : "AG_PKG_ERROR_PATH",
                "SFML_DIR" : "AG_PKG_ERROR_PATH",
                "TGUI_DIR" : "AG_PKG_ERROR_PATH",
                "BOX2D_DIR" : "AG_PKG_ERROR_PATH"
            },
            "lldb.launch.env": {
                "LD_LIBRARY_PATH":"AG_PKG_ERROR_PATH"
            }
        }
    )"_json;
    vs_code_settings["cmake.configureSettings"]["BOOST_ROOT"] = boost_root.string();
    vs_code_settings["cmake.configureSettings"]["SFML_DIR"] = sfml_cmake_dir.string();
    vs_code_settings["cmake.configureSettings"]["TGUI_DIR"] = tgui_cmake_dir.string();
    vs_code_settings["cmake.configureSettings"]["BOX2D_DIR"] = box2d_root.string();
    vs_code_settings["lldb.launch.env"]["LD_LIBRARY_PATH"] = sfml_lib_dir.string() + ":" + tgui_lib_dir.string();

    // write to file
    std::ofstream settings_out_file(settings_filepath.string(), std::ofstream::out | std::ofstream::app);
    if(settings_out_file.good())
    {
        settings_out_file << vs_code_settings.dump(4);
    }
    else
    {
        std::cout << "Bad Settings file path" << std::endl;
    }
}

int main()
{
    // poco setup
    SSLInitializer sslInitializer;
	Poco::Net::HTTPStreamFactory::registerFactory();
	Poco::Net::HTTPSStreamFactory::registerFactory();
    Poco::Net::FTPStreamFactory::registerFactory();
    // Note: we must create the passphrase handler prior Context
	Poco::SharedPtr ptrCert(new Poco::Net::ConsoleCertificateHandler(false)); // ask the user via console
	Poco::Net::Context::Ptr ptrContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", "", "", Poco::Net::Context::VERIFY_STRICT, 9, true, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
	Poco::Net::SSLManager::instance().initializeClient(0, ptrCert, ptrContext);

    //Define a form.
    nana::form fm {nullptr, {400, 300}, nana::form::appear::decorate<nana::form::appear::maximize>()};
    fm.caption("AG PKG");
    fm.events().unload([]{
        nana::API::exit_all();
    });

    // status
    nana::label status_label{fm, ""};

    // Install directory
    nana::label download_dir_lbl{fm, "Install Directory"};
    download_dir_lbl.format(true);
    std::string default_install_path = "~/AG/Dependencies/";
    nana::textbox install_dir_textbox {fm, default_install_path};
    install_dir_textbox.multi_lines(false);
    nana::button search_for_install_directory_btn {fm, "Search"};
    search_for_install_directory_btn.events().click([&fm, &install_dir_textbox]()
    {
        nana::folderbox folder_box;
        auto folder = folder_box.show();
        if (folder)
        {
            install_dir_textbox.select(true);
            install_dir_textbox.del();
            install_dir_textbox.append(folder.value().string(), false);
        }
    });

    // Project directory
    nana::label project_dir_lbl{fm, "Project Directory"};
    project_dir_lbl.format(true);
    std::string default_project_dir = "~/AG/Projects/MyAGProject";
    nana::textbox project_dir_textbox {fm, default_project_dir};
    project_dir_textbox.multi_lines(false);
    nana::button search_for_project_directory_btn {fm, "Search"};
    search_for_project_directory_btn.events().click([&fm, &project_dir_textbox]()
    {
        nana::folderbox folder_box;
        auto folder = folder_box.show();
        if (folder)
        {
            project_dir_textbox.select(true);
            project_dir_textbox.del();
            project_dir_textbox.append(folder.value().string(), false);
        }
    });

    // Install button
    nana::button download_btn{fm, "Install"};
    download_btn.events().click([&]{
        std::cout << "downloading..." << std::endl;
        status_label.caption("downloading...");
        boost::filesystem::path final_install_path(install_dir_textbox.caption());
        // ensure the path exists
        boost::filesystem::create_directories(final_install_path);
        // safely append AG_Dependencies.zip to the filepath
        boost::filesystem::path final_unzip_path = final_install_path / "AG_Dependencies_1_0_0";
        final_install_path /= "AG_Dependencies_1_0_0.zip";
        auto final_install_path_string = final_install_path.string();

        // install
        download_file("https://www.dropbox.com/s/w18c6uoqxvm9gfh/Clang_7_stdlibcpp.zip?dl=1", final_install_path_string);

        status_label.caption("unzipping...");
        unzip_file_to_dest(final_install_path_string, final_unzip_path.string());
        
        // project setup
        status_label.caption("creating project...");
        create_vsc_proj_in_folder(project_dir_textbox.caption(), final_unzip_path.string());
        status_label.caption("done");
    });

    // layout form
    fm.div(
        "vert"\
        "<>"\
        "<margin=[0, 0, 0, 5] download_dir_lbl>"\
        "<"\
            "weight=15%"\
            "<weight=80% margin=[0, 5, 0, 5] download_dir_file_path>"\
            "<margin=[0, 5, 0, 0] search_for_install_directory_btn>"\
        ">"\
        "<>"\
        "<margin=[0, 0, 0, 5] project_dir_lbl>"\
        "<"\
            "weight=15%"\
            "<weight=80% margin=[0, 5, 0, 5] project_dir_file_path>"\
            "<margin=[0, 5, 0, 0] search_for_project_directory_btn>"\
        ">"\
        "<>"\
        "<>"\
        "<download_btn>"
        "<status_label>"\
    );

    // add widgets to the form
    fm["download_dir_lbl"] << download_dir_lbl;
    fm["download_dir_file_path"] << install_dir_textbox;
    fm["project_dir_lbl"] << project_dir_lbl;
    fm["project_dir_file_path"] << project_dir_textbox;
    fm["download_btn"] << download_btn;
    fm["search_for_install_directory_btn"] << search_for_install_directory_btn;
    fm["search_for_project_directory_btn"] << search_for_project_directory_btn;
    fm["status_label"] << status_label;
    fm.collocate();

    // show form
    fm.show();

    // Start to event loop process, it blocks until the form is closed.
    nana::exec();
    return 0;
}