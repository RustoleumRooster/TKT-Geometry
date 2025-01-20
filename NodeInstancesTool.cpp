
#include <irrlicht.h>
#include <iostream>
#include "NodeInstancesTool.h"
#include "reflected_nodes.h"
#include "Reflection.h"
#include "GUI_tools.h"
#include "ex_gui_elements.h"
#include "edit_env.h"
#include "edit_classes.h"
#include "geometry_scene.h"
#include "node_properties.h"
#include <sstream>

using namespace irr;
using namespace gui;

extern IrrlichtDevice* device;

Node_Instances_Base* Node_Instances_Tool::base = NULL;
Node_Instances_Base* Node_Selector_Tool::base = NULL;
multi_tool_panel* Node_Instances_Tool::panel = NULL;

REFLECT_CUSTOM_STRUCT_BEGIN(node_instance)
	REFLECT_STRUCT_MEMBER(id)
	REFLECT_STRUCT_MEMBER(name)
	REFLECT_STRUCT_MEMBER(node_ptr)
REFLECT_STRUCT_END()

REFLECT_CUSTOM_STRUCT_BEGIN(node_tree_item)
	REFLECT_STRUCT_MEMBER(item_type)
	REFLECT_STRUCT_MEMBER(id)
	REFLECT_STRUCT_MEMBER(class_name)
	REFLECT_STRUCT_MEMBER(instances)
	REFLECT_TREE_STRUCT_MEMBERS()
REFLECT_STRUCT_END()

DEFINE_TREE_FUNCTIONS(node_tree_item)


void node_instance::write_attributes(reflect::Member* m_struct)
{
	node_instance::my_typeDesc* m_type = (node_instance::my_typeDesc*)(m_struct->type);

	m_type->my_attributes.selected = this->selected;
}

void node_tree_item::write_attributes(reflect::Member* m_struct)
{
	m_struct->expanded = this->expanded;
	node_tree_item::my_typeDesc* m_type = (node_tree_item::my_typeDesc*)(m_struct->type);

	((node_tree_item::my_typeDesc*)(m_struct->type))->hasSubs = this->hasSubs;

	for (int i = 0; i < instances.size(); i++)
	{
		reflect::Member* M = &m_type->members[3];
		instances[i].write_attributes(&((node_instance::my_typeDesc*)(M->type))->members[i]);
	}
}

node_tree_item* node_tree_item::find_type(reflect::TypeDescriptor_Struct* tD)
{
	if (this->typeDescriptor == tD)
		return this;

	else for (node_tree_item& sub : sub_classes)
	{
		node_tree_item* res = sub.find_type(tD);
		if (res)
			return res;
	}

	return NULL;
}

bool node_tree_item::has_items_recursive()
{
	bool b = this->instances.size() > 0;

	for (int i = 0; i < sub_classes.size(); i++)
	{
		if (sub_classes[i].has_items_recursive())
			b = true;
	}

	hasSubs = b;

	return hasSubs;
}

void node_instance::my_typeDesc::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree, size_t offset_base, bool bVisible, bool bEditable, int tab)
{
	int m_i = tree[tree.size() - 1];
	std::string name = type_struct->members[m_i].name;
	size_t offset = type_struct->members[m_i].offset + offset_base;


	String_StaticField* f = new String_StaticField();

	f->setText(type_struct->members[m_i].name);
	f->init("             ", tree, offset + ALIGN_BYTES, tab, bVisible);

	if (my_attributes.selected)
		//f->bBorder = true;
		f->bHighlight = true;

	f->bCanSelect = true;
	win->addEditField(f);
}

void node_tree_item::my_typeDesc::addFormWidget(Reflected_GUI_Edit_Form* win, TypeDescriptor_Struct* type_struct, std::vector<int> tree_0, size_t offset_base, bool bVisible, bool bEditable, int tab)
{
	TREE_STRUCT_ADD_FORMWIDGET_HEADER()

		if (!hasSubs)
			return;

		if (type_struct != NULL)
		{
			size_t offset = type_struct->members[m_i].offset + offset_base;

			String_StaticField* f = new String_StaticField();

			f->setText(type_struct->members[m_i].name);
			f->init("             ", tree_0, offset + ALIGN_BYTES*2, tab, bVisible);
			
			f->text_color = FORM_TEXT_COLOR_GREY;

			win->addEditField(f);
			int sub_pos = 3;

			std::vector<int> tree_1 = tree_0;
			tree_1.push_back(sub_pos);
			reflect::TypeDescriptor_Struct* td_struct = (reflect::TypeDescriptor_Struct*)members[sub_pos].type;
			size_t new_offset = type_struct->members[m_i].offset + offset_base + members[sub_pos].offset;
			for (int j = 0; j < td_struct->members.size(); j++) 
			{
					std::vector<int> tree = tree_1;
					tree.push_back(j);
					td_struct->members[j].type->addFormWidget(win, td_struct, tree, new_offset, type_struct->members[m_i].expanded && bVisible, bEditable, tab + 1);
			} 
		}

	TREE_STRUCT_ADD_FORMWIDGET_FOOTER()
}

Node_Instances_Widget::Node_Instances_Widget(gui::IGUIEnvironment* env, gui::IGUIElement* parent, geometry_scene* g_scene_, Node_Instances_Base* base_, s32 id, core::rect<s32> rect)
	: gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rect), my_base(base_), g_scene(g_scene_), my_ID(id)
{
	MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
	receiver->Register(this);
}


Node_Instances_Widget::~Node_Instances_Widget()
{
	//std::cout << "Out of scope (Node Instances Widget)\n";
	MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();
	receiver->UnRegister(this);
}


void Node_Instances_Widget::show()
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
	std::string str = my_base->getTypesString();
	std::wstring s(str.begin(), str.end());

	my_static_text = Environment->addStaticText(s.c_str(),sr,false,false,this);

	edit_panel->drop();
	my_widget->drop();
}

void Node_Instances_Widget::onRefresh()
{
	if (my_button)
		my_button->remove();

	if (my_static_text)
		my_static_text->remove();

	int ypos = my_widget->getEditAreaHeight() + 8;
	core::rect<s32> br = core::rect<s32>(getRelativePosition().getWidth() - 80, ypos, getRelativePosition().getWidth() - 8, ypos + 36);
	
	my_button = new Flat_Button(Environment, this, OK_BUTTON_ID, br);
	my_button->setText(L"Ok");
	my_button->drop();

	core::rect<s32> sr = core::rect<s32>(4, ypos, getRelativePosition().getWidth() - 90, ypos + 24);
	std::string str = my_base->getTypesString();
	std::wstring s(str.begin(), str.end());

	my_static_text = Environment->addStaticText(s.c_str(), sr, false, false, this);
}


bool Node_Instances_Widget::OnEvent(const SEvent& event)
{
	if (event.EventType == EET_USER_EVENT)
	{
		if (event.UserEvent.UserData1 == USER_EVENT_REFLECTED_FORM_REFRESHED)
		{
			onRefresh();
			return true;
		}
		else if (event.UserEvent.UserData1 == USER_EVENT_SELECTION_CHANGED)
		{
			my_base->select();

			my_widget->refresh();
			onRefresh();

			return true;
		}
	}
	else if (event.EventType == EET_GUI_EVENT)
	{
		s32 id = event.GUIEvent.Caller->getID();

		if (event.GUIEvent.EventType == EGET_BUTTON_CLICKED ||
			event.GUIEvent.EventType == (gui::EGUI_EVENT_TYPE)GUI_BUTTON_SHIFT_CLICKED)
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
						leaf.push_back(2);	//grab the struct member in the 3rd position

						reflect::Member* m = my_widget->m_typeDesc->getTreeNode(leaf);
						size_t offset = my_widget->m_typeDesc->getTreeNodeOffset(leaf);

						char* sel;
						m->type->copy(&sel, (char*)((char*)my_widget->temp_object) + offset);

						if (event.GUIEvent.EventType == (gui::EGUI_EVENT_TYPE)GUI_BUTTON_SHIFT_CLICKED)
						{
							my_base->select((Reflected_SceneNode*)sel, true);
						}
						else
						{
							my_base->select((Reflected_SceneNode*)sel, false);
						}

						return true;
					}
				}
			}
		}
	}

	return IGUIElement::OnEvent(event);
}

void Node_Instances_Widget::click_OK()
{
	MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();

	SEvent event;
	event.EventType = EET_USER_EVENT;
	event.UserEvent.UserData1 = USER_EVENT_NODES_SELECTED;
	receiver->OnEvent(event);

	if (Parent)
	{
		SEvent e;
		e.EventType = EET_GUI_EVENT;
		e.GUIEvent.Caller = this;
		e.GUIEvent.EventType = (gui::EGUI_EVENT_TYPE)GUI_REFLECTED_FORM_CLOSED;
		Parent->OnEvent(e);
	}
}

void Node_Instances_Base::init_member(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
	reflect::Member* m = flat_typeDescriptor->getTreeNode(tree_pos);
	m->readwrite = true;
}

void Node_Instances_Base::toggle_expanded_struct(reflect::TypeDescriptor_Struct* flat_typeDescriptor, std::vector<int> tree_pos)
{
	reflected_tool_base::toggle_expanded_struct(flat_typeDescriptor, tree_pos);
	m_struct.read_expanded(flat_typeDescriptor, std::vector<int>{});
}

std::string Node_Instances_Base::getTypesString()
{
	if (g_scene->getSelectedNodes().size() == 0)
		return std::string("Selection: none");
	else
	{
		std::vector<reflect::TypeDescriptor_Struct*> tds = Node_Properties_Base::GetTypeDescriptors(g_scene);
		reflect::TypeDescriptor_Struct* td = tds[tds.size() - 1];

		std::stringstream ss;
		ss << "Selection: " << g_scene->getSelectedNodes().size() << " " << td->alias;
		return std::string(ss.str());
	}
}

void Node_Instances_Base::set_selected(node_instance& item)
{
	item.selected = false;
	for (Reflected_SceneNode* n : g_scene->getSelectedNodes())
	{
		if (item.node_ptr == (char*)n)
			item.selected = true;
	}
}

void Node_Instances_Base::select()
{
	m_struct.set_selected_recursive(this);
}

void Node_Instances_Base::select(Reflected_SceneNode* sel, bool shift)
{

	std::vector<int> old_sel_faces = g_scene->getSelectedFaces();
	std::vector<Reflected_SceneNode*> old_sel_nodes = g_scene->getSelectedNodes();
	std::vector<int> old_sel_brushes = g_scene->getBrushSelection();

	if (shift)
		g_scene->setSelectedNodes_ShiftAdd(sel);
	else
	{
		g_scene->setSelectedNodes(std::vector<Reflected_SceneNode*>{ sel });
	}

	g_scene->setSelectedFaces(std::vector<int>{});
	g_scene->setBrushSelection(std::vector<int>{});

	if (old_sel_faces == g_scene->getSelectedFaces() &&
		old_sel_nodes == g_scene->getSelectedNodes() &&
		old_sel_brushes == g_scene->getBrushSelection())
	{
		return;
	}

	g_scene->selectionChanged();
}

reflect::TypeDescriptor_Struct* Node_Instances_Base::getFlatTypeDescriptor()
{
	reflect::TypeDescriptor_Struct* tD = (reflect::TypeDescriptor_Struct*)m_typeDescriptor->get_flat_copy(getObj(), 0);

	//m_struct.write_attributes_recursive(tD, std::vector<int>{});

	return tD;
}

void Node_Instances_Base::write_attributes(reflect::TypeDescriptor_Struct* flat_typeDescriptor)
{
	m_struct.write_attributes_recursive(flat_typeDescriptor, std::vector<int>{});
}

void Node_Instances_Base::widget_closing(Reflected_Widget_EditArea* widget)
{
	m_initial_struct.read_expanded(widget->m_typeDesc, std::vector<int>{});
}

Node_Instances_Base::Node_Instances_Base(std::wstring name, int my_id, gui::IGUIEnvironment* env, multi_tool_panel* panel)
	: simple_reflected_tool_base(name,my_id,env,panel)
{
	m_typeDescriptor = (reflect::TypeDescriptor_Struct*)reflect::TypeResolver<node_tree_item>::get();

	build_initial_struct();
}

void Node_Instances_Base::show()
{
	build_struct();

	core::rect<s32> client_rect(core::vector2di(0, 0),
		core::dimension2du(this->panel->getClientRect()->getAbsolutePosition().getWidth(),
			this->panel->getClientRect()->getAbsolutePosition().getHeight()));

	Node_Instances_Widget* widget = new Node_Instances_Widget(env, this->panel->getClientRect(), g_scene, this, my_ID, client_rect);

	widget->show();
	widget->drop();
}

void Node_Instances_Base::show_window()
{
	build_struct();

	Node_Selector_Window* win = new Node_Selector_Window(env, env->getRootGUIElement(), this, g_scene, -1, core::rect<s32>(500, 64, 500 + 400, 64 + 340));

	std::wstring s(L"Select Nodes");
	win->setText(s.c_str());

	win->show();
	win->drop();

	Node_Instances_Widget* widget = new Node_Instances_Widget(env, win, g_scene, this, my_ID, win->getClientRect());

	widget->show();
	widget->drop();
}

void Node_Instances_Base::build_initial_struct()
{
	std::vector<reflect::TypeDescriptor_Struct*> all_classes = Reflected_SceneNode_Factory::getAllTypes();

	m_initial_struct.sub_classes.clear();
	int index;
	for (index = 0; index < all_classes.size(); index++)
	{
		if (all_classes[index]->inherited_type == NULL)
		{
			node_tree_item nc;
			nc.id = index;
			nc.typeDescriptor = all_classes[index];
			nc.class_name = std::string(nc.typeDescriptor->alias);
			nc.expanded = true;

			m_initial_struct.sub_classes.push_back(nc);

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
			for (int j = 0; j < m_initial_struct.sub_classes.size(); j++)
			{
				node_tree_item* res = m_initial_struct.sub_classes[j].find_type(all_classes[index]->inherited_type);
				if (res)
				{
					node_tree_item nc;
					nc.id = index;
					nc.typeDescriptor = all_classes[index];
					nc.class_name = std::string(nc.typeDescriptor->alias);
					nc.expanded = true;
					res->sub_classes.push_back(nc);

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

void Node_Instances_Base::build_struct()
{
	m_struct = m_initial_struct;

	int i = 0;
	for(ISceneNode* it : g_scene->EditorNodes()->getChildren())
	{
		Reflected_SceneNode* node = (Reflected_SceneNode*)it;
		reflect::TypeDescriptor_Struct* typeDesc = node->GetDynamicReflection();

		node_tree_item* res = m_struct.find_type(typeDesc);
		if (res)
		{
			node_instance ni;
			ni.id = i;
			ni.node_ptr = (char*)node;
			ni.name = std::string(typeDesc->alias);
			ni.selected = node->bSelected;
			res->instances.push_back(ni);
			i++;
		}
	}

	m_struct.has_items_recursive();
}

Node_Selector_Window::Node_Selector_Window(gui::IGUIEnvironment* env, gui::IGUIElement* parent, Node_Instances_Base* base, geometry_scene* g_scene_, s32 id, core::rect<s32> rect)
	: gui::CGUIWindow(env, parent, id, rect), my_base(base)
{
}

Node_Selector_Window::~Node_Selector_Window()
{
	MyEventReceiver* receiver = (MyEventReceiver*)device->getEventReceiver();

	SEvent event;
	event.EventType = EET_USER_EVENT;
	event.UserEvent.UserData1 = USER_EVENT_NODE_SELECTION_ENDED;

	receiver->OnEvent(event);
}

void Node_Selector_Window::show()
{
}

bool Node_Selector_Window::OnEvent(const SEvent& event)
{
	if (event.EventType == EET_USER_EVENT)
	{
		return false;
	}
	else if (event.EventType == EET_GUI_EVENT)
	{
		if (event.GUIEvent.EventType == (gui::EGUI_EVENT_TYPE)GUI_REFLECTED_FORM_CLOSED)
		{
			remove();
			return true;
		}
	}

	return gui::CGUIWindow::OnEvent(event);
}

void node_tree_item::set_selected_recursive(Node_Instances_Base* base)
{
	for (node_instance& item : instances)
		base->set_selected(item);

	for (node_tree_item& item : sub_classes)
		item.set_selected_recursive(base);
}
