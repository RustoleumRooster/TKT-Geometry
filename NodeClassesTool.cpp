
#include <irrlicht.h>
#include <iostream>
#include "NodeClassesTool.h"
#include "reflected_nodes.h"
#include "Reflection.h"
#include "GUI_tools.h"
#include "ex_gui_elements.h"
#include "edit_env.h"
#include "edit_classes.h"
#include <sstream>
#include "geometry_scene.h"

using namespace irr;
using namespace gui;

Node_Classes_Base* Node_Classes_Tool::base = NULL;
multi_tool_panel* Node_Classes_Tool::panel = NULL;


REFLECT_CUSTOM_STRUCT_BEGIN(node_class_item)
	REFLECT_STRUCT_MEMBER(id)
	REFLECT_STRUCT_MEMBER(class_name)
	REFLECT_TREE_STRUCT_MEMBERS()
REFLECT_STRUCT_END()

DEFINE_TREE_FUNCTIONS(node_class_item)

void node_class_item::write_attributes(reflect::Member* m_struct)
{
	m_struct->expanded = this->expanded;

	node_class_item::my_typeDesc* m_type = (node_class_item::my_typeDesc*)(m_struct->type);

	m_type->hasSubs = this->sub_classes.size() > 0;
	m_type->my_attributes.placeable = typeDescriptor->placeable;
	m_type->my_attributes.selected = selected;
}

node_class_item* node_class_item::find_type(reflect::TypeDescriptor_Struct* tD)
{
	if (this->typeDescriptor == tD)
		return this;

	else for (node_class_item& sub : sub_classes)
	{
		node_class_item* res = sub.find_type(tD);
		if (res)
			return res;
	}

	return NULL;
}

void node_class_item::set_selected_recursive(Node_Classes_Base* base)
{
	base->set_selected(this);

	for (node_class_item& item : sub_classes)
		item.set_selected_recursive(base);
}

void node_class_item::my_typeDesc::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree_0, size_t offset_base, bool bVisible, bool bEditable, int tab)
{
	TREE_STRUCT_ADD_FORMWIDGET_HEADER()

	if (type_struct != NULL)
		{
			size_t offset = type_struct->members[m_i].offset + offset_base;

			String_StaticField* f = new String_StaticField();

			f->setText(type_struct->members[m_i].name);
			f->init("             ", tree_0, offset + ALIGN_BYTES, tab, bVisible);
			f->bCanSelect = true;

			if (my_attributes.placeable == false)
			{
				f->text_color = FORM_TEXT_COLOR_GREY;
				f->bCanSelect = false;
			}
			if (my_attributes.selected)
			{
				f->bHighlight = true;
			}

			win->addEditField(f);
		}

	TREE_STRUCT_ADD_FORMWIDGET_FOOTER()
}

Node_Classes_Widget::Node_Classes_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene* g_scene_, Node_Classes_Base* base_, s32 id, core::rect<s32> rect)
	: gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rect), my_base(base_), g_scene(g_scene_), my_ID(id)
{
	//MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
	//receiver->Register(this);
}


Node_Classes_Widget::~Node_Classes_Widget()
{
	std::cout << "Out of scope (Node Classes Widget)\n";
	//MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
	//receiver->UnRegister(this);
}


void Node_Classes_Widget::show()
{
	core::rect<s32> pr(0, 0, getRelativePosition().getWidth(), getRelativePosition().getHeight());
	edit_panel = new gui::IGUIElement(gui::EGUIET_ELEMENT, Environment, this, -1, pr);

	OK_BUTTON_ID = my_ID + 1;
	STATIC_TEXT_ID = OK_BUTTON_ID + 1;

	my_widget = new Reflected_Widget_EditArea(Environment, edit_panel, g_scene, my_base, STATIC_TEXT_ID + 1, pr);
	my_widget->show(false, my_base->getObj());

	int ypos = my_widget->getEditAreaHeight() + 8;
	core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

	my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
	my_button->setText(L"Ok");
	my_button->drop();

	core::rect<s32> sr = core::rect<s32>(4, ypos, getRelativePosition().getWidth() - 90, ypos + 24);
	std::string str = my_base->getStaticString();
	std::wstring s(str.begin(), str.end());

	my_static_text = Environment->addStaticText(s.c_str(), sr, false, false, this);

	edit_panel->drop();
	my_widget->drop();
}

void Node_Classes_Widget::onRefresh()
{
	if (my_button)
		my_button->remove();

	if (my_static_text)
		my_static_text->remove();


	int ypos = my_widget->getEditAreaHeight() + 8;
	core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

	my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
	my_button->setText(L"Ok");

	core::rect<s32> sr = core::rect<s32>(4, ypos, getRelativePosition().getWidth() - 90, ypos + 24);
	std::string str = my_base->getStaticString();
	std::wstring s(str.begin(), str.end());

	my_static_text = Environment->addStaticText(s.c_str(), sr, false, false, this);
}

void Node_Classes_Widget::refresh()
{
	if (my_widget)
	{
		my_widget->remove();
	}

	core::rect<s32> pr(0, 0, getRelativePosition().getWidth(), getRelativePosition().getHeight());
	my_widget = new Reflected_Widget_EditArea(Environment, edit_panel, g_scene, my_base, my_ID + 2, pr);

	my_base->build_struct();

	my_widget->show(false, my_base->getObj());
	my_widget->drop();

	onRefresh();
}

bool Node_Classes_Widget::OnEvent(const SEvent& event)
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

						return true;
					}
				}
			}
		}
	}

	return IGUIElement::OnEvent(event);
}

void Node_Classes_Widget::click_OK()
{
	//reflect::TypeDescriptor_Struct* td = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<geo_settings_struct>::get();

	//my_widget->write(td, my_base->getObj());
}

void Node_Classes_Base::init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
	reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);
	m->readwrite = true;
}

Node_Classes_Base::Node_Classes_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel) :
	simple_reflected_tool_base(name, my_id, env, panel) 
{
	m_typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<node_class_item>::get();

	build_struct();
}

std::string Node_Classes_Base::getStaticString()
{
	if (selected_type)
	{
		std::stringstream ss;
		ss << "Selection: " << selected_type->alias;
		return std::string(ss.str());
	}

	return std::string("Selection: none");
}

void Node_Classes_Base::select(int sel)
{
	reflect::TypeDescriptor_Struct* td = Reflected_SceneNode_Factory::getAllTypes()[sel];
	selected_type = td;

	m_struct.set_selected_recursive(this);
}

void Node_Classes_Base::set_scene(geometry_scene* g_scene_)
{
	tool_base::set_scene(g_scene_);

	g_scene->set_node_classes_base(this);

	build_struct();
}

reflect::TypeDescriptor_Struct* Node_Classes_Base::getFlatTypeDescriptor()
{
	reflect::TypeDescriptor_Struct* tD = (reflect::TypeDescriptor_Struct*)m_typeDescriptor->get_flat_copy(getObj(), 0);
	return tD;
}

void Node_Classes_Base::write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor)
{
	m_struct.write_attributes_recursive(flat_typeDescriptor, std::vector<int>{});
}

void Node_Classes_Base::toggle_expanded_struct(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
	reflected_tool_base::toggle_expanded_struct(flat_typeDescriptor, tree_pos);
	m_struct.read_expanded(flat_typeDescriptor, std::vector<int>{});
}

void Node_Classes_Base::set_selected(node_class_item* item)
{
	if (item->typeDescriptor == selected_type)
		item->selected = true;
	else item->selected = false;
}

void Node_Classes_Base::widget_closing(Reflected_Widget_EditArea* widget)
{
	//m_typeDescriptor->dump(&m_struct, 0);
	//for (reflect::Member m : m_typeDescriptor->members)
	//	std::cout << m.name << " " << m.offset << "\n";
	//std::cout << "\n";
	//m_struct.read_expanded(widget->m_typeDesc, std::vector<int>{});
	m_struct.read_expanded(m_typeDescriptor, std::vector<int>{});

}

void Node_Classes_Base::show()
{
	//m_typeDescriptor->dump(&m_struct, 0);

	core::rect<s32> client_rect(core::vector2di(0, 0),
		core::dimension2du(this->panel->getClientRect()->getAbsolutePosition().getWidth(),
			this->panel->getClientRect()->getAbsolutePosition().getHeight()));

	Node_Classes_Widget* widget = new Node_Classes_Widget(env, this->panel->getClientRect(), g_scene, this, my_ID, client_rect);

	widget->show();
	widget->drop();
}

void Node_Classes_Base::build_struct()
{
	std::vector<reflect::TypeDescriptor_Struct*> all_classes = Reflected_SceneNode_Factory::getAllTypes();

	m_struct.sub_classes.clear();
	int index;
	for (index = 0; index < all_classes.size(); index++)
	{

		if (all_classes[index]->inherited_type == NULL)
		{
			node_class_item nc;
			nc.typeDescriptor = (reflect::TypeDescriptor_SN_Struct*) all_classes[index];
			nc.id = Reflected_SceneNode_Factory::getTypeNum(nc.typeDescriptor);
			nc.class_name = std::string(nc.typeDescriptor->alias);
			nc.expanded = true;
			nc.selected = nc.typeDescriptor == selected_type;
			m_struct.sub_classes.push_back(nc);

			std::vector<reflect::TypeDescriptor_Struct*> new_classes;
			for (int i = 0; i < all_classes.size(); i++)
			{
				if (i != index)
					new_classes.push_back(all_classes[i]);
			}
			all_classes = new_classes;
			break;
		}
	}

	while (all_classes.size() > 0)
	{
		for (index = 0; index < all_classes.size(); index++)
		{
			for (int j = 0; j < m_struct.sub_classes.size(); j++)
			{
				node_class_item* res = m_struct.sub_classes[j].find_type(all_classes[index]->inherited_type);
				if (res)
				{
					node_class_item nc;
					
					nc.typeDescriptor = (reflect::TypeDescriptor_SN_Struct*) all_classes[index];
					nc.id = Reflected_SceneNode_Factory::getTypeNum(nc.typeDescriptor);
					nc.class_name = std::string(nc.typeDescriptor->alias);
					nc.expanded = true;
					nc.selected = nc.typeDescriptor == selected_type;
					nc.placeable = ((reflect::TypeDescriptor_SN_Struct*)nc.typeDescriptor)->placeable;
					res->sub_classes.push_back(nc);
					
					//std::cout << all_classes[index]->name << " is under " << res->typeDescriptor->name << "\n";

					std::vector<reflect::TypeDescriptor_Struct*> new_classes;
					for (int i = 0; i < all_classes.size(); i++)
					{
						if (i != index)
						{
							new_classes.push_back(all_classes[i]);
						}
					}
					all_classes = new_classes;
					goto try_again;

				}
			}
		}
	try_again:
		int a = 0;
	}
}
