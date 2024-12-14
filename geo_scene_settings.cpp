#include <irrlicht.h>
#include "geometry_scene.h"
#include "geo_scene_settings.h"
#include "edit_env.h"
#include "ex_gui_elements.h"
#include <sstream>
#include "BufferManager.h"
#include "material_groups.h"

using namespace irr;

extern IrrlichtDevice* device;

Geo_Settings_Base* Geo_Settings_Tool::base = NULL;
multi_tool_panel* Geo_Settings_Tool::panel = NULL;

int snap_sizes[] = { 1,2,4,8,16,32,64,128,256 };

REFLECT_MULTI_STRUCT_BEGIN(drag_snap_sizes_struct)
	REFLECT_MULTI_STRUCT_LABEL("1")
	REFLECT_MULTI_STRUCT_LABEL("2")
	REFLECT_MULTI_STRUCT_LABEL("4")
	REFLECT_MULTI_STRUCT_LABEL("8")
	REFLECT_MULTI_STRUCT_LABEL("16")
	REFLECT_MULTI_STRUCT_LABEL("32")
	REFLECT_MULTI_STRUCT_LABEL("64")
	REFLECT_MULTI_STRUCT_LABEL("128")
	REFLECT_MULTI_STRUCT_LABEL("256")
REFLECT_MULTI_STRUCT_END()

f32 rotate_snap_sizes[] = { 1,2.5,5,15,45 };

REFLECT_MULTI_STRUCT_BEGIN(rotate_snap_sizes_struct)
	REFLECT_MULTI_STRUCT_LABEL("1")
	REFLECT_MULTI_STRUCT_LABEL("2.5")
	REFLECT_MULTI_STRUCT_LABEL("5")
	REFLECT_MULTI_STRUCT_LABEL("15")
	REFLECT_MULTI_STRUCT_LABEL("45")
REFLECT_MULTI_STRUCT_END()


REFLECT_STRUCT_BEGIN(editor_settings_struct)
	REFLECT_STRUCT_MEMBER(snap_dist)
	REFLECT_STRUCT_MEMBER(rotate_snap)
	REFLECT_STRUCT_MEMBER(bAutoSave)
REFLECT_STRUCT_END()


REFLECT_STRUCT_BEGIN(build_settings_struct)
	REFLECT_STRUCT_MEMBER(bProgressiveBuild)
	REFLECT_STRUCT_MEMBER(bBuildFinal)
	REFLECT_STRUCT_MEMBER(bAutoRebuildMeshes)
	//REFLECT_STRUCT_MEMBER(bOptimizeTriangles)
REFLECT_STRUCT_END()


REFLECT_STRUCT_BEGIN(geo_settings_struct)
	REFLECT_STRUCT_MEMBER(editor_settings)
	REFLECT_STRUCT_MEMBER(build_settings)
	REFLECT_STRUCT_MEMBER(IntArray)
	REFLECT_STRUCT_MEMBER(FloatArray)
REFLECT_STRUCT_END()

Geo_Settings_Widget::Geo_Settings_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene* g_scene_, Geo_Settings_Base* base_, s32 id, core::rect<s32> rect)
	: gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rect), my_base(base_), g_scene(g_scene_)
{
	//MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
	//receiver->Register(this);
}


Geo_Settings_Widget::~Geo_Settings_Widget()
{
	//std::cout << "Out of scope (Geo Settings Widget)\n";

	//MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
	//receiver->UnRegister(this);

}

void Geo_Settings_Widget::show()
{
	core::rect<s32> pr(0, 0, getRelativePosition().getWidth(), getRelativePosition().getHeight());
	edit_panel = new gui::IGUIElement(gui::EGUIET_ELEMENT, Environment, this, -1, pr);

	OK_BUTTON_ID = my_ID + 1;

	my_widget = new Reflected_Widget_EditArea(Environment, edit_panel, g_scene, my_base, my_ID + 2, pr);
	//my_widget->setName("material groups widget");

	//reflect::TypeDescriptor_Struct* typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<geo_settings_struct>::get();

	my_widget->setColumns(std::vector<int>{60,60,60});

	my_widget->show(true, my_base->getObj());

	int ypos = my_widget->getEditAreaHeight()+8;
	core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

	my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
	my_button->setText(L"Save");

	edit_panel->drop();
	my_widget->drop();
}

void Geo_Settings_Widget::onRefresh()
{
	if (my_button)
		my_button->remove();

	int ypos = my_widget->getEditAreaHeight() + 8;
	core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

	my_button = new Flat_Button(Environment, this, OK_BUTTON_ID,br);
	my_button->setText(L"Save");
}

bool Geo_Settings_Widget::OnEvent(const SEvent& event)
{
	if (event.EventType == EET_USER_EVENT)
	{
		if (event.UserEvent.UserData1 == USER_EVENT_REFLECTED_FORM_REFRESHED)
		{
			onRefresh();
			return true;
		}
	}
	else if (event.EventType == EET_GUI_EVENT)
	{
		s32 id = event.GUIEvent.Caller->getID();

		if (event.GUIEvent.EventType == EGET_BUTTON_CLICKED)
		{
			if (event.GUIEvent.Caller == my_button)
			{
				click_OK();
				return true;
			}
		}
	}

	return IGUIElement::OnEvent(event);
}


void Geo_Settings_Widget::click_OK()
{
	//reflect::TypeDescriptor_Struct* td = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<geo_settings_struct>::get();

	my_widget->write();
}



//============================================================
//
//


void Geo_Settings_Base::show()
{

	core::rect<s32> client_rect(core::vector2di(0, 0),
		core::dimension2du(this->panel->getClientRect()->getAbsolutePosition().getWidth(),
			this->panel->getClientRect()->getAbsolutePosition().getHeight()));

	Geo_Settings_Widget* widget = new Geo_Settings_Widget(env, this->panel->getClientRect(), g_scene, this, GUI_ID_GEO_SETTINGS_BASE, client_rect);

	widget->show();
	widget->drop();
}


