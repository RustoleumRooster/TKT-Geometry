
#include <irrlicht.h>
#include <iostream>
#include "NodeTools.h"
#include "reflected_nodes.h"
#include "Reflection.h"
#include "GUI_tools.h"
#include "ex_gui_elements.h"
#include "edit_env.h"
#include "edit_classes.h"

using namespace irr;


Node_Classes_Base* Node_Classes_Tool::base = NULL;
multi_tool_panel* Node_Classes_Tool::panel = NULL;


REFLECT_CUSTOM_STRUCT_BEGIN(node_class_item)
	REFLECT_STRUCT_MEMBER(id)
	REFLECT_STRUCT_MEMBER(class_name)
	REFLECT_STRUCT_MEMBER(expanded)
	REFLECT_STRUCT_MEMBER(sub_classes)
REFLECT_STRUCT_END()


REFLECT_STRUCT_BEGIN(node_classes_struct)
	REFLECT_STRUCT_MEMBER(nClasses)
	REFLECT_STRUCT_MEMBER(classes)
REFLECT_STRUCT_END()


void node_class_item::my_typeDesc::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree_0, size_t offset_base, bool bVisible, bool bEditable, int tab)
{
	std::cout << std::string(4 * (tab + 1), ' ') << "node_class_item ";
	for (int i = 0; i < tree_0.size(); i++)
		std::cout << tree_0[i] << " ";
	std::cout << " [" << offset_base << "]\n";

	int m_i = tree_0.size() > 0 ? tree_0[tree_0.size() - 1] : 0;

	int vector_pos = 3;
	bool hasSubs = members.size() > vector_pos;

	if (type_struct != NULL && hasSubs)
	{
		ExButton_FormField* ff = new ExButton_FormField();

		ff->tree_pos = tree_0;
		ff->text = type_struct->members[m_i].expanded ? "-" : "+";
		ff->tab = (tab == -1 ? 0 : tab);
		ff->setVisible(bVisible);
		win->addEditField(ff);
	}

	if (type_struct != NULL)
	{
		//Text_ExpandableLabel* f = new Text_ExpandableLabel();
		//Text_StaticField* f = new Text_StaticField();
		size_t offset = type_struct->members[m_i].offset + offset_base;

		String_StaticField* f = new String_StaticField();

		f->setText(type_struct->members[m_i].name);
		f->init("             ", tree_0, offset + sizeof(int), tab, bVisible);
		f->bCanSelect = true;

		win->addEditField(f);
	}

	if (type_struct == NULL && hasSubs)
		for (int i = vector_pos; i < members.size(); i++)
		{
			std::vector<int> tree_1 = tree_0;
			tree_1.push_back(i);
			reflect::TypeDescriptor_Struct* td_struct = (reflect::TypeDescriptor_Struct*)members[vector_pos].type;
			size_t new_offset = members[vector_pos].offset;

			for (int j = 0; j < td_struct->members.size(); j++)
			{
				std::vector<int> tree = tree_1;
				tree.push_back(j);
				td_struct->members[j].type->addFormWidget(win, td_struct, tree, new_offset, bVisible, bEditable, tab + 1);
			}
		}
	
	if (type_struct != NULL && hasSubs)
	{
		for (int i = vector_pos; i < members.size(); i++)
		{
			std::vector<int> tree_1 = tree_0;
			tree_1.push_back(i);
			reflect::TypeDescriptor_Struct* td_struct = (reflect::TypeDescriptor_Struct*)members[vector_pos].type;
			size_t new_offset = type_struct->members[m_i].offset + offset_base + members[vector_pos].offset;
			
			for (int j = 0; j < td_struct->members.size(); j++)
			{
				std::vector<int> tree = tree_1;
				tree.push_back(j);
				td_struct->members[j].type->addFormWidget(win, td_struct, tree, new_offset, type_struct->members[m_i].expanded && bVisible, bEditable, tab + 1);
			}
		
		}
	}
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

	my_widget = new Reflected_Widget_EditArea(Environment, edit_panel, g_scene, my_base, my_ID + 2, pr);
	//my_widget->setName("material groups widget");

	//reflect::TypeDescriptor_Struct* typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<Node_Classes_struct>::get();

	my_widget->show(false, my_base->getObj());

	int ypos = my_widget->getEditAreaHeight() + 8;
	core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

	my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
	my_button->setText(L"Ok");

	edit_panel->drop();
	my_widget->drop();
}

void Node_Classes_Widget::onRefresh()
{
	if (my_button)
		my_button->remove();

	int ypos = my_widget->getEditAreaHeight() + 8;
	core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);

	my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
	my_button->setText(L"Ok");
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

void Node_Classes_Base::initialize(std::wstring name_, int my_id, gui::IGUIEnvironment* env_, geometry_scene* g_scene_, multi_tool_panel* panel_)
{
	tool_base::initialize(name_, my_id, env_, g_scene_, panel_);
	m_typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<node_class_item>::get();

	build_struct();

}

void Node_Classes_Base::show()
{

	m_typeDescriptor->dump(&m_struct, 0);

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
	//m_struct.nClasses = all_classes.size();

	m_struct.sub_classes.clear();
	int index;
	for (index = 0; index < all_classes.size(); index++)
	{

		if (all_classes[index]->inherited_type == NULL)
		{
			node_class_item nc;
			nc.id = index;
			nc.typeDescriptor = all_classes[index];
			nc.class_name = std::string(nc.typeDescriptor->alias);
			nc.expanded = true;
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
		std::cout << all_classes.size() << " remaining\n";
		for (index = 0; index < all_classes.size(); index++)
		{
			for (int j = 0; j < m_struct.sub_classes.size(); j++)
			{
				node_class_item* res = m_struct.sub_classes[j].find_type(all_classes[index]->inherited_type);
				if (res)
				{
					node_class_item nc;
					nc.id = index;
					nc.typeDescriptor = all_classes[index];
					nc.class_name = std::string(nc.typeDescriptor->alias);
					nc.expanded = true;
					res->sub_classes.push_back(nc);

					std::cout << all_classes[index]->name << " is under " << res->typeDescriptor->name << "\n";

					std::vector<reflect::TypeDescriptor_Struct*> new_classes;
					for (int i = 0; i < all_classes.size(); i++)
					{
						if (i != index)
							new_classes.push_back(all_classes[i]);
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