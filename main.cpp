#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/filebox.hpp>
#include <nana/gui/widgets/textbox.hpp>

#include <iostream>
#include <string>

int main()
{
    using namespace nana;

    //Define a form.
    form fm;
    fm.caption("AG PKG");

    // Install directory
    label download_dir_lbl{fm, "Install Directory"};
    download_dir_lbl.format(true);
    std::string default_install_path = "~/AG/Dependencies/";
     // folderbox install_folderbox {fm, default_install_path};
     textbox install_folderbox {fm, default_install_path};


    // Project directory
    label project_dir_lbl{fm, "Project Directory"};
    project_dir_lbl.format(true);
    std::string default_project_dir = "~/AG/Projects/MyAGProject";
    // folderbox project_folderbox {fm, default_project_dir};
    textbox project_folderbox {fm, default_project_dir};

    // Define a button and answer the click event.
    button download_btn{fm, "Install"};
    download_btn.events().click([]{
        std::cout << "downloading..." << std::endl;
    });

    //Layout management
    // fm.div("vert <><<><weight=80% text><>><><weight=24<><button><>><>");
    // fm["text"]<<lab;
    // fm["button"] << btn;
    // fm.collocate();

    fm.div("vert <><download_dir_lbl><download_dir_file_path><><project_dir_lbl><project_dir_file_path><><download_btn>");
    fm["download_dir_lbl"] << download_dir_lbl;
    fm["download_dir_file_path"] << install_folderbox;
    fm["project_dir_lbl"] << project_dir_lbl;
    fm["project_dir_file_path"] << project_folderbox;
    fm["download_btn"] << download_btn;
    fm.collocate();
	
    //Show the form
    fm.show();

    //Start to event loop process, it blocks until the form is closed.
    exec();
}