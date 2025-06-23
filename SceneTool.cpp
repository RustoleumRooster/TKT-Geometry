#include <irrlicht.h>
#include "SceneTool.h"
#include "geometry_scene.h"
#include "GUI_tools.h"
#include "edit_classes.h"
#include "edit_env.h"
#include "ex_gui_elements.h"

extern SceneCoordinator* gs_coordinator;
extern IrrlichtDevice* device;
extern IEventReceiver* ContextMenuOwner;

using namespace irr;
using namespace gui;

Scene_Instances_Base* Scene_Instances_Tool::base = NULL;
multi_tool_panel* Scene_Instances_Tool::panel = NULL;

REFLECT_CUSTOM_STRUCT_BEGIN(scene_instance_item)
	REFLECT_STRUCT_MEMBER(id)
	REFLECT_STRUCT_MEMBER(scene_name)
	REFLECT_TREE_STRUCT_MEMBERS()
REFLECT_STRUCT_END()

DEFINE_TREE_FUNCTIONS(scene_instance_item)

void scene_instance_item::write_attributes(reflect::Member* m_struct)
{
	m_struct->expanded = this->expanded;

	scene_instance_item::my_typeDesc* m_type = (scene_instance_item::my_typeDesc*)(m_struct->type);

	m_type->hasSubs = this->sub_classes.size() > 0;
	m_type->my_attributes.selected = selected;
	m_type->my_attributes.editable = editable;
}

void scene_instance_item::my_typeDesc::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree_0, size_t offset_base, bool bVisible, bool bEditable, int tab)
{
	TREE_STRUCT_ADD_FORMWIDGET_HEADER()

		if (type_struct != NULL)
		{
			size_t offset = type_struct->members[m_i].offset + offset_base;

			String_FormField* f;

			if (my_attributes.editable)
			{
				f = new String_EditField();
				//f->bAskForFocus = true;
			}
			else
				f = new String_StaticField();

			f->setText(type_struct->members[m_i].name);
			f->init("             ", tree_0, offset + ALIGN_BYTES, tab, bVisible);
			f->bCanSelect = true;

			if (my_attributes.selected)
			{
				f->bHighlight = true;
			}

			win->addEditField(f);
		}

	TREE_STRUCT_ADD_FORMWIDGET_FOOTER()
}

void scene_instance_item::set_selected_recursive(Scene_Instances_Base* base)
{
	base->set_selected(this);

	for (scene_instance_item& item : sub_classes)
		item.set_selected_recursive(base);
}

//===================================================
// Scene Instances Widget
//

Scene_Instances_Widget::Scene_Instances_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, Scene_Instances_Base* base_, s32 id, core::rect<s32> rect)
	: gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rect), my_base(base_), my_ID(id)
{
	//MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
	//receiver->Register(this);
}


Scene_Instances_Widget::~Scene_Instances_Widget()
{
	std::cout << "Out of scope (Scene Instances Widget)\n";
	//MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
	//receiver->UnRegister(this);
}


void Scene_Instances_Widget::show()
{
	core::rect<s32> pr(0, 0, getRelativePosition().getWidth(), getRelativePosition().getHeight());
	edit_panel = new gui::IGUIElement(gui::EGUIET_ELEMENT, Environment, this, -1, pr);

	OK_BUTTON_ID = my_ID + 1;
	//STATIC_TEXT_ID = OK_BUTTON_ID + 1;

	my_widget = new Reflected_Widget_EditArea(Environment, edit_panel, NULL, my_base, my_ID + 2, pr);
	my_widget->show(true, my_base->getObj());

	int ypos = my_widget->getEditAreaHeight() + 8;
	core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

	my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
	my_button->setText(L"Add Scene");
	my_button->drop();

	//core::rect<s32> sr = core::rect<s32>(4, ypos, getRelativePosition().getWidth() - 90, ypos + 24);
	//std::string str = my_base->getStaticString();
	//std::wstring s(str.begin(), str.end());

	//my_static_text = Environment->addStaticText(s.c_str(), sr, false, false, this);

	edit_panel->drop();
	my_widget->drop();
}


void Scene_Instances_Widget::onRefresh()
{
	if (my_button)
		my_button->remove();

	//if (my_static_text)
	//	my_static_text->remove();


	int ypos = my_widget->getEditAreaHeight() + 8;
	core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

	my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
	my_button->setText(L"Add Scene");

	//core::rect<s32> sr = core::rect<s32>(4, ypos, getRelativePosition().getWidth() - 90, ypos + 24);
	//std::string str = my_base->getStaticString();
	//std::wstring s(str.begin(), str.end());

	//my_static_text = Environment->addStaticText(s.c_str(), sr, false, false, this);
}

void Scene_Instances_Widget::refresh()
{
	if (my_widget)
	{
		my_widget->remove();
	}

	core::rect<s32> pr(0, 0, getRelativePosition().getWidth(), getRelativePosition().getHeight());
	my_widget = new Reflected_Widget_EditArea(Environment, edit_panel, NULL, my_base, my_ID + 1, pr);

	my_base->build_struct();

	my_widget->show(false, my_base->getObj());
	my_widget->drop();

	onRefresh();
}


bool Scene_Instances_Widget::OnEvent(const SEvent& event)
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
						int sel = read_formfield_id(field);

						my_base->select(sel);

						my_widget->refresh();
						onRefresh();

						return true;
					}
				}
			}
		}
		else if (event.GUIEvent.EventType == (gui::EGUI_EVENT_TYPE)GUI_BUTTON_RIGHT_CLICKED)
		{
			FormField* field = my_widget->form->getFieldFromId(id);

			//std::cout << id << " id\n";
			editing_element_id = id;

			if (field)
			{
				int sel = read_formfield_id(field);

				core::vector2di pos = event.GUIEvent.Caller->getAbsolutePosition().UpperLeftCorner;
				pos.X += 64;

				ContextMenuOwner = this;

				my_base->right_click_item(pos,sel);

				return true;
			}
		}
		else if (event.GUIEvent.EventType == EGET_MENU_ITEM_SELECTED)
		{

			gui::IGUIContextMenu* menu = (gui::IGUIContextMenu*)event.GUIEvent.Caller;
			s32 id = menu->getItemCommandId(menu->getSelectedItem());

			if (id == GUI_ID_SCENE_INSTANCES_RIGHTCLICK_MENU_ITEM_RENAME)
			{
				my_base->rename_item();
			}
			else if (id == GUI_ID_SCENE_INSTANCES_RIGHTCLICK_MENU_ITEM_DELETE)
			{
				my_base->delete_item();
			}

			my_widget->refresh();
			onRefresh();

			FormField* f = my_widget->form->edit_fields;
			while (f)
			{
				if (f->my_ID == editing_element_id)
				{
					//std::cout << "found it ! " << f->my_ID << "\n";
					Environment->setFocus( f->getStaticElement(0));
				}
				f = f->next;
			}


			return true;
		}
		else if (event.GUIEvent.EventType == EGET_ELEMENT_FOCUSED)
		{
			if (event.GUIEvent.Element &&
				(event.GUIEvent.Element->getType() == gui::EGUIET_EDIT_BOX))
			{
				//std::cout << "rename success!\n";

				my_widget->write_by_field();

				my_base->right_click_item(vector2di{}, -1);

				my_widget->refresh();
				onRefresh();
			}
		}
	}

	return IGUIElement::OnEvent(event);
}

int Scene_Instances_Widget::read_formfield_id(FormField* field)
{
	std::vector<int> leaf = field->tree_pos;
	leaf.push_back(0); //node_class_item.id

	reflect::Member* m = my_widget->m_typeDesc->getTreeNode(leaf);
	size_t offset = my_widget->m_typeDesc->getTreeNodeOffset(leaf);

	int sel;

	m->type->copy(&sel, (char*)((char*)my_widget->temp_object) + offset);

	return sel;
}

void Scene_Instances_Widget::click_OK()
{
	gs_coordinator->add_scene();
	refresh();
}

//===================================================
// Scene Instances Base
//

Scene_Instances_Base::Scene_Instances_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel) :
	simple_reflected_tool_base(name, my_id, env, panel)
{
	m_typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<scene_instance_item>::get();

	build_struct();
}

reflect::TypeDescriptor_Struct* Scene_Instances_Base::getFlatTypeDescriptor()
{
	reflect::TypeDescriptor_Struct* tD = (reflect::TypeDescriptor_Struct*)m_typeDescriptor->get_flat_copy(getObj(), 0);
	return tD;
}

void Scene_Instances_Base::select(int sel)
{
	//reflect::TypeDescriptor_Struct* td = Reflected_SceneNode_Factory::getAllTypes()[sel];
	if (selection == sel)
		return;

	selection = sel;

	std::cout << "swapping to scene " << sel << "\n";

	gs_coordinator->swap_scene(sel);

	m_struct.set_selected_recursive(this);
}

void Scene_Instances_Base::write_obj_by_field(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos, void* obj)
{
	std::vector<int> leaf = tree_pos;
	leaf.push_back(1);

	int p_i = tree_pos[1];

	size_t offset = flat_typeDescriptor->getTreeNodeOffset(leaf);

	if (flat_typeDescriptor->getTreeNode(tree_pos)->modified)
	{
		std::string new_name = *(std::string*)((char*)obj + offset);
		gs_coordinator->scenes[p_i]->rename(new_name);
	}

	std::cout << m_struct.sub_classes[p_i].scene_name << " renamed to "<< gs_coordinator->scenes[p_i]->name() <<"\n";
}

void Scene_Instances_Base::delete_item()
{

}

void Scene_Instances_Base::rename_item()
{
	bNameEditing = true;

	m_struct.set_selected_recursive(this);
}

void Scene_Instances_Base::right_click_item(core::vector2di pos, int sel)
{
	if (sel == -1)
	{
		bNameEditing = false;
		m_struct.set_selected_recursive(this);
		return;
	}

	rightClickedItem = sel;

	IGUIEnvironment* environment = device->getGUIEnvironment();
	gui::IGUIContextMenu* menu = environment->addContextMenu(core::rect<s32>(pos, core::vector2di(256, 256)), 0, -1);
	menu->addItem(L"Rename", GUI_ID_SCENE_INSTANCES_RIGHTCLICK_MENU_ITEM_RENAME, true, false, false, false);
	menu->addItem(L"Delete", GUI_ID_SCENE_INSTANCES_RIGHTCLICK_MENU_ITEM_DELETE, true, false, false, false);

}

void Scene_Instances_Base::init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
	reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);
	m->readwrite = true;
}

void Scene_Instances_Base::write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor)
{
	m_struct.write_attributes_recursive(flat_typeDescriptor, std::vector<int>{});
}

void Scene_Instances_Base::toggle_expanded_struct(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
	//reflected_tool_base::toggle_expanded_struct(flat_typeDescriptor, tree_pos);
	//m_struct.read_expanded(flat_typeDescriptor, std::vector<int>{});
}

void Scene_Instances_Base::set_selected(scene_instance_item* item)
{
	if (item->id == selection)
	{
		item->selected = true;
	}
	else
	{
		item->selected = false;
	}

	if (bNameEditing && item->id == rightClickedItem)
	{
		item->editable = true;
	}
	else
	{
		item->editable = false;
	}
}

void Scene_Instances_Base::build_struct()
{
	m_struct.sub_classes.clear();
	m_struct.id = -1;
	m_struct.scene_name = "All scenes";

	for (int i = 0; i < gs_coordinator->scenes.size(); i++)
	{
		scene_instance_item item;
		item.id = i;
		item.scene_name = gs_coordinator->scenes[i]->name();
		m_struct.sub_classes.push_back(item);
	}

	selection = gs_coordinator->scene_no;
}


void Scene_Instances_Base::show()
{
	//m_typeDescriptor->dump(&m_struct, 0);

	core::rect<s32> client_rect(core::vector2di(0, 0),
		core::dimension2du(this->panel->getClientRect()->getAbsolutePosition().getWidth(),
			this->panel->getClientRect()->getAbsolutePosition().getHeight()));

	Scene_Instances_Widget* widget = new Scene_Instances_Widget(env, this->panel->getClientRect(), this, my_ID, client_rect);

	build_struct();

	widget->show();
	widget->drop();
}

void Scene_Instances_Base::widget_closing(Reflected_Widget_EditArea* widget)
{
	//m_typeDescriptor->dump(&m_struct, 0);
	//for (reflect::Member m : m_typeDescriptor->members)
	//	std::cout << m.name << " " << m.offset << "\n";
	//std::cout << "\n";
	//m_struct.read_expanded(widget->m_typeDesc, std::vector<int>{});
	//m_struct.read_expanded(m_typeDescriptor, std::vector<int>{});

}