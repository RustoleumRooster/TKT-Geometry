#ifndef _TKT_FILE_OPEN_H_
#define _TKT_FILE_OPEN_H_

#include <irrlicht.h>
#include "CGUIWindow.h"
#include "reflect_custom_types.h"
#include "GUI_tools.h"

using namespace irr;

class Flat_Button;
class Reflected_Widget_EditArea;
class File_Open_Base;
class geometry_scene;

namespace reflect
{
    struct TypeDescriptor_Struct;
}

template class simple_reflected_tree_tool_base<folder_tree_item>;

class File_Open_Widget : public gui::IGUIElement
{
public:
    File_Open_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene*, File_Open_Base*, s32 id, core::rect<s32> rect);
    ~File_Open_Widget();

    void show();
    void refresh();
    void onRefresh();
    virtual void click_OK();
    virtual bool OnEvent(const SEvent& event);



    int OK_BUTTON_ID = 0;
    int STATIC_TEXT_ID = 0;
    int EDIT_BOX_ID = 0;
    int my_ID = 0;

    Reflected_Widget_EditArea* my_widget = NULL;
    Flat_Button* my_button = NULL;
    gui::IGUIStaticText* my_static_text = NULL;
    gui::IGUIEditBox* my_editbox = NULL;

    geometry_scene* g_scene = NULL;
    File_Open_Base* my_base = NULL;
    gui::IGUIElement* edit_panel = NULL;
};



class File_Open_Window : public gui::CGUIWindow
{
public:
    File_Open_Window(gui::IGUIEnvironment* env, gui::IGUIElement* parent, File_Open_Base* my_base, geometry_scene* g_scene_, s32 id, core::rect<s32> rect);
    ~File_Open_Window();

    void show();
    //void refresh();

    virtual bool OnEvent(const SEvent& event);
    File_Open_Base* my_base = NULL;
};


class File_Open_Base: public simple_reflected_tree_tool_base<folder_tree_item>
{
public:

    File_Open_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel);

    virtual void show();

    //virtual void initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_);

    void pathToStringW(irr::core::stringw& result, const irr::io::path& p);

    std::string getWorkingDirStr();
    void widget_closing(Reflected_Widget_EditArea*);

    void setStartDir(io::path p) {
        StartDirectory = p;
    }

    void setWindowText(std::string txt) {
        windowText = txt;
    }

    void setFileName(const irr::io::path& name);
    void setFileNameOnly(const irr::io::path& name);
    void setDirectoryName(const irr::io::path& name);

    void build_struct();

    void select_file(int sel);
    void select_file_dbl(int sel);

    void click_OK();
    void window_closed();

    core::stringw getFileBaseNameW();
    void setFileBaseNameW(core::stringw);

    void SetCurrentProjectPath(io::path);
    io::path GetCurrentProjectPath();
    bool ProjectPathLoaded() { return project_path_loaded; }

private:

    io::path current_project_path;
    bool project_path_loaded = false;

    io::path FileName;
    core::stringw FileNameW;
    core::stringw FileBaseNameW;
    io::path FileDirectory;
    io::path FileDirectoryFlat;
    core::stringw FileDirectoryFlatW;
    io::path RestoreDirectory;
    io::path StartDirectory;
    io::path SelectedPath;

    bool restoreCWD = true;

    io::IFileSystem* FileSystem = NULL;
    io::IFileList* FileList = NULL;

    std::string windowText;

    friend class File_Open_Tool;
};


class File_Open_Tool
{
    static File_Open_Base* base;
    static multi_tool_panel* panel;

public:

    static void show(const char* windowtext)
    {
        base->setWindowText(std::string(windowtext));
        base->show();
        //panel->add_tool(base);
    }

    static void initialize(File_Open_Base* base_, multi_tool_panel* panel_)
    {
        base = base_;
        panel = panel_;
    }

    static File_Open_Base* get_base()
    {
        return base;
    }

    static io::path getSelectedFile();
    static io::path getSelectedDir();
};



#endif