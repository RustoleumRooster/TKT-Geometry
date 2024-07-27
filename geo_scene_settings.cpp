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

Material_Buffers_Base* Material_Buffers_Tool::base = NULL;
multi_tool_panel* Material_Buffers_Tool::panel = NULL;

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

REFLECT_CUSTOM_STRUCT_BEGIN(material_group_struct)
	REFLECT_STRUCT_MEMBER(id)
	REFLECT_STRUCT_MEMBER(nFaces)
	REFLECT_STRUCT_MEMBER(nTriangles)
	REFLECT_STRUCT_MEMBER(texture)
	REFLECT_STRUCT_MEMBER(material_group)
REFLECT_STRUCT_END()

REFLECT_STRUCT_BEGIN(material_buffers_struct)
	REFLECT_STRUCT_MEMBER(nBuffers)
	REFLECT_STRUCT_MEMBER(material_groups)
REFLECT_STRUCT_END()


void material_group_struct::my_typeDesc::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree, size_t offset_base, bool bVisible, bool bEditable, int tab)
{
	
	int m_i = tree[tree.size() - 1];
	std::string name = type_struct->members[m_i].name;
	size_t offset = type_struct->members[m_i].offset + offset_base;

	if(m_i==0)
	{
		Text_StaticField* f = new Text_StaticField();
		f->initInline("faces", tree, offset, 1, bVisible);
		win->addEditField(f);

		f = new Text_StaticField();
		f->initInline("triangles", tree, offset, 2, bVisible);
		win->addEditField(f);

		f = new Text_StaticField();
		f->init(" ", tree, offset, tab, bVisible);
		win->addEditField(f);
	}
	

	{
		Int_StaticField* f = new Int_StaticField();
		f->initInline("", tree, offset + 8, 1, bVisible);
		f->bBorder = true;
		win->addEditField(f);

		f = new Int_StaticField();
		f->initInline("", tree, offset + 16, 2, bVisible);
		f->bBorder = true;
		win->addEditField(f);
	}

	Text_StaticField* f = new Text_StaticLabel();
	
	//String_StaticField* f = new String_StaticField();
	f->bCanSelect = true;

	std::stringstream ss;
	ss << "buffer " << m_i;
	f->setText(ss.str().c_str());

	f->tab = (tab == -1 ? 0 : tab);
	f->setVisible(bVisible);
	f->tree_pos = tree;

	if (my_attributes.selected)
	{
		f->bHighlight = true;
	}

	win->addEditField(f);

	//TypeDescriptor_Struct::addFormWidget(win, type_struct, obj, tree, offset_base, bVisible, bEditable, tab);
	
}



Material_Buffers_Widget::Material_Buffers_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene* g_scene_, Material_Buffers_Base* base_, s32 id, core::rect<s32> rect)
	: gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rect), my_base(base_), g_scene(g_scene_), my_ID(id)
{
	MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
	receiver->Register(this);
}


Material_Buffers_Widget::~Material_Buffers_Widget()
{
	//std::cout << "Out of scope (Material Buffers Widget)\n";

	MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
	receiver->UnRegister(this);

	my_base->close_uv_panel();

}


void Material_Buffers_Widget::show()
{
	core::rect<s32> pr(0, 0, getRelativePosition().getWidth(), getRelativePosition().getHeight());
	edit_panel = new gui::IGUIElement(gui::EGUIET_ELEMENT, Environment, this, -1, pr);

	OK_BUTTON_ID = my_ID + 1;

	my_widget = new Reflected_Widget_EditArea(Environment, edit_panel, g_scene, my_base, my_ID + 2, pr);
	//my_widget->setName("material groups widget");

	//reflect::TypeDescriptor_Struct* typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<material_buffers_struct>::get();

	my_widget->show(false, my_base->getObj());

	int ypos = my_widget->getEditAreaHeight() + 8;

	core::rect<s32> pr2(0, ypos, getRelativePosition().getWidth(), ypos+120);

	options_form = new Reflected_GUI_Edit_Form(Environment, edit_panel, g_scene, my_ID + 3, pr2);
	reflect::TypeDescriptor_Struct* options_td = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<mb_tool_options_struct>::get();
	
	options_td->addFormWidget(options_form, NULL, vector<int>{0}, 0, true, true, 0);
	options_form->ShowWidgets(my_ID + 4);
	options_form->read(my_base->getOptions());

	ypos += my_widget->getFormsHeight();
	core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

	my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
	my_button->setText(L"Ok");

	edit_panel->drop();
	my_widget->drop();

}

void Material_Buffers_Widget::onRefresh()
{
	//Display options
	options_form->remove();

	int ypos = my_widget->getEditAreaHeight() + 8;

	core::rect<s32> pr2(0, ypos, getRelativePosition().getWidth(), ypos + 120);

	options_form = new Reflected_GUI_Edit_Form(Environment, edit_panel, g_scene, my_ID + 3, pr2);
	reflect::TypeDescriptor_Struct* options_td = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<mb_tool_options_struct>::get();

	options_td->addFormWidget(options_form, NULL, vector<int>{0}, 0, true, true, 0);
	options_form->ShowWidgets(my_ID + 4);
	options_form->read(my_base->getOptions());

	ypos += options_form->getTotalHeight() + 8;

	//Button
	if (my_button)
		my_button->remove();

	core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

	my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
	my_button->setText(L"Ok");

	//Text
	if (my_text)
		my_text->remove();

	core::rect<s32> tr = core::rect<s32>(32, ypos, getRelativePosition().getWidth() - 80, ypos + 24);

	std::wstringstream ss;
	ss << my_base->GetSelectedString().c_str();
	my_text = Environment->addStaticText(ss.str().c_str(), tr, false, false, this);

	ypos += 32;

	//Image
	if (my_image)
		my_image->remove();

	core::rect<s32> ir = core::rect<s32>(32, ypos, 32+128, ypos + 128);

	video::ITexture* texture = my_base->GetSelectedTexture();

	if (texture)
	{
		my_image = Environment->addImage(ir, this, -1);
		my_image->setScaleImage(true);
		my_image->setImage(texture);
	}
}

bool Material_Buffers_Widget::OnEvent(const SEvent& event)
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
			else
			{
				FormField* field = my_widget->form->getFieldFromId(id);

				if (field)
				{
					if (field->getButtonType() == FORM_FIELD_LABEL)
					{

						std::vector<int> leaf = field->tree_pos;
						leaf.push_back(0); //node_class_item.id

						reflect::Member* m = my_widget->m_typeDesc->getTreeNode(leaf);
						size_t offset = my_widget->m_typeDesc->getTreeNodeOffset(leaf);

						int sel;

						m->type->copy(&sel, (char*)((char*)my_widget->temp_object) + offset);

						my_base->select(sel);

						my_widget->refresh();
						onRefresh();

						g_scene->set_selected_material_group(sel);

						MyEventReceiver* event_receiver = (MyEventReceiver*)device->getEventReceiver();
						SEvent event;
						event.EventType = EET_USER_EVENT;
						event.UserEvent.UserData1 = USER_EVENT_MATERIAL_GROUP_SELECTION_CHANGED;
						event_receiver->OnEvent(event);
						
						return true;
					}
				}
			}
			return true;
		}
		else if (event.GUIEvent.EventType == EGET_CHECKBOX_CHANGED)
		{
			options_form->write(my_base->getOptions());
			my_base->refresh_panel_view();
		}
	}

	return IGUIElement::OnEvent(event);
}

void Material_Buffers_Widget::click_OK()
{
	//reflect::TypeDescriptor_Struct* td = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<geo_settings_struct>::get();

	//my_widget->write(td, my_base->getObj());
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


//============================================================
//
//

void Material_Buffers_Base::initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_, scene::ISceneManager* smgr)
{
	tool_base::initialize(name_, my_id, env_, g_scene_, panel_);
	m_typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<material_buffers_struct>::get();

	uv_edit = new UV_Editor_Panel(env, device->getVideoDriver(), NULL, NULL, 0, core::recti());
	uv_edit->Initialize(smgr, g_scene);
}

void Material_Buffers_Base::show()
{
	refresh_panel_view();

	std::vector<TextureMaterial> materials = g_scene->final_meshnode_interface.getMaterialsUsed();
	m_struct.nBuffers = materials.size();

	m_struct.material_groups.clear();
	for (int i = 0; i < m_struct.nBuffers; i++)
	{
		material_group_struct mgs;

		mgs.id = i;
		mgs.nTriangles = materials[i].n_triangles;
		mgs.nFaces = materials[i].n_faces;
		mgs.texture = materials[i].texture;
		mgs.material_group = materials[i].materialGroup;
		mgs.selected = false;
		m_struct.material_groups.push_back(mgs);
	}

	core::rect<s32> client_rect(core::vector2di(0, 0),
		core::dimension2du(this->panel->getClientRect()->getAbsolutePosition().getWidth(),
			this->panel->getClientRect()->getAbsolutePosition().getHeight()));

	Material_Buffers_Widget* widget = new Material_Buffers_Widget(env, this->panel->getClientRect(), g_scene, this, GUI_ID_MAT_BUFFERS_BASE, client_rect);

	widget->show();
	widget->drop();
}

void Material_Buffers_Base::select(int sel)
{
	selection = sel;

	for (int i = 0; i < m_struct.material_groups.size(); i++)
	{
		if (i == sel)
			m_struct.material_groups[i].selected = true;
		else
			m_struct.material_groups[i].selected = false;
	}

}

void Material_Buffers_Base::close_uv_panel()
{
	if (uv_edit && uv_edit->getViewPanel())
		uv_edit->getViewPanel()->disconnect();
}

void Material_Buffers_Base::refresh_panel_view()
{
	if (cameraQuad && uv_edit)
	{
		if (mb_options.show_uv_view && uv_edit->hooked_up() == false)
		{
			cameraQuad->hookup_aux_panel(uv_edit);
		}
		else if (mb_options.show_uv_view == false && uv_edit->hooked_up() == true)
		{
			uv_edit->getViewPanel()->disconnect();
		}

		if (uv_edit->hooked_up() == true)
		{
			uv_edit->showTriangles(mb_options.show_triangles);
			uv_edit->showLightmap(mb_options.show_lightmap);

			MyEventReceiver* event_receiver = (MyEventReceiver*)device->getEventReceiver();
			SEvent event;
			event.EventType = EET_USER_EVENT;
			event.UserEvent.UserData1 = USER_EVENT_MATERIAL_GROUP_SELECTION_CHANGED;
			event_receiver->OnEvent(event);
		}

	}
}

void Material_Buffers_Base::init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
	reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);
	//material_group_struct::my_typeDesc* m_type = (material_group_struct::my_typeDesc*)(m->type);
	m->readwrite = true;

	if (tree_pos.size() > 1)
	{
		//m_type->my_attributes.selected = m_struct.material_groups[tree_pos[1]].selected;
	}
}

void Material_Buffers_Base::write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor)
{
	for (int i = 0; i < m_struct.material_groups.size(); i++)
	{
		std::vector<int> tree_pos{1, i};

		reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);

		material_group_struct::my_typeDesc* m_type = (material_group_struct::my_typeDesc*)(m->type);
		m_type->my_attributes.selected = m_struct.material_groups[i].selected;
	}
}

std::string Material_Buffers_Base::GetSelectedString()
{
	if (selection != -1)
	{
		int mg = m_struct.material_groups[selection].material_group;
		return Material_Groups_Tool::get_base()->getMaterialGroup(mg).name;
	}
	return std::string();
}

video::ITexture* Material_Buffers_Base::GetSelectedTexture()
{
	if (selection != -1)
	{
		return m_struct.material_groups[selection].texture;

	}

	return nullptr;
}

REFLECT_STRUCT_BEGIN(mb_tool_options_struct)
	REFLECT_STRUCT_MEMBER(show_uv_view)
	REFLECT_STRUCT_MEMBER(show_triangles)
	REFLECT_STRUCT_MEMBER(show_lightmap)
REFLECT_STRUCT_END()