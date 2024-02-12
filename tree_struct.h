#ifndef _TREE_STRUCT_H_
#define _TREE_STRUCT_H_

#include <irrlicht.h>
#include "Reflection.h"
#include <vector>

using namespace irr;

//===================================================
// TREE STRUCT
// 
// Macros to define a helpful class, a reflected struct that has an std::vector
// of its own type as a member, so can have a tree-structure. It also has a boolean
// variable that tells whether to expand the tree when displayed as a widget. Main purpose
// is to create on the fly tree-structure GUI widgets.
//

#define REFLECT_TREE_STRUCT_MEMBERS() \
	REFLECT_STRUCT_MEMBER(expanded) \
	REFLECT_STRUCT_MEMBER(sub_classes) \
	T::bool_pos = typeDesc->members.size()-2; \
	T::vec_pos = typeDesc->members.size()-1;


#define DECLARE_TREE_FUNCTIONS(tree_type) \
	bool expanded = false; \
	std::vector<tree_type> sub_classes; \
    tree_type* find_node(std::vector<int> tree_pos); \
    void write_attributes_recursive(reflect::TypeDescriptor_Struct* tD, std::vector<int> tree_0); \
	void write_attributes(reflect::Member* m_struct);\
    void read_expanded(reflect::TypeDescriptor_Struct* tD, std::vector<int> tree_0, int tab);


#define REFLECT_CUSTOM_TREE_STRUCT() \
	static u32 bool_pos; \
    static u32 vec_pos; \
	struct my_typeDesc : reflect::TypeDescriptor_Struct { \
		bool hasSubs=true; \
		attributes my_attributes;\
        my_typeDesc(void (*init)(reflect::TypeDescriptor_Struct*)) : reflect::TypeDescriptor_Struct(init) {} \
        virtual void addFormWidget(Reflected_GUI_Edit_Form* win, reflect::TypeDescriptor_Struct* type_struct, std::vector<int> tree, size_t offset, bool bVisible, bool bEditable, int tab); \
        virtual reflect::TypeDescriptor* get_flat_copy(void* obj, int indent) { \
            return TypeDescriptor_Struct::get_flat_copy(obj,indent); \
            } \
        virtual reflect::TypeDescriptor_Struct* getNewEmptyStruct();\
        }; \
    friend struct reflect::DefaultResolver; \
    static my_typeDesc Reflection; \
    static void initReflection(reflect::TypeDescriptor_Struct*);


#define DEFINE_TREE_FUNCTIONS(tree_type) \
	u32 tree_type::bool_pos = 0; \
	u32 tree_type::vec_pos = 0; \
	\
	tree_type* tree_type::find_node(std::vector<int> tree_pos) { \
		if (tree_pos.size() == 0) \
			return this; \
		tree_type* n = this; \
		for (int i = 0; i < tree_pos.size(); i++) { \
			n = &n->sub_classes[tree_pos[i]]; \
		} \
		return n; \
	} \
	\
	void tree_type::write_attributes_recursive(reflect::TypeDescriptor_Struct* tD, std::vector<int> tree_0) { \
		reflect::Member* m_struct = tD->getTreeNode(tree_0); \
		if (m_struct) { \
			this->write_attributes(m_struct); \
		} \
		for (int i = 0; i < this->sub_classes.size(); i++) { \
			std::vector<int> tree = tree_0; \
			tree.push_back(vec_pos); \
			tree.push_back(i); \
			this->sub_classes[i].write_attributes_recursive(tD, tree); \
		} \
	} \
	void tree_type::read_expanded(reflect::TypeDescriptor_Struct* tD, std::vector<int> tree_0, int tab = 0) { \
		std::vector<int> tree_b = tree_0; \
		/*std::cout << std::string(3 * (tab + 1), ' ')<<" ";*/\
		for(int i=0; i<tree_b.size(); i++) /*std::cout << tree_b[i] <<" "; std::cout<<", subs = ";*/\
		tree_b.push_back(tree_type::bool_pos); \
		reflect::Member* m_struct = tD->getTreeNode(tree_0); \
		reflect::Member* m_bool = tD->getTreeNode(tree_b); \
		if (m_struct && m_bool){ \
			bool bExpanded = m_struct->expanded; \
			*(bool*)m_bool->get(this) = bExpanded; \
		} \
		/*std::cout<<sub_classes.size()<<":\n";*/\
		for (int i = 0; i < sub_classes.size(); i++) { \
			std::vector<int> tree = tree_0; \
			tree.push_back(tree_type::vec_pos); \
			tree.push_back(i); \
			sub_classes[i].read_expanded(tD, tree, tab+1); \
		} \
	}
/*\
	void tree_type::read_expanded(reflect::TypeDescriptor_Struct* tD, std::vector<int> tree_0, int tab = 0) { \
		std::vector<int> tree_b = tree_0; \
		std::cout << std::string(3 * (tab + 1), ' ')<<" ";\
		for(int i=0; i<tree_b.size(); i++) std::cout << tree_b[i] <<" "; std::cout<<", subs = ";\
		tree_b.push_back(tree_type::bool_pos); \
		reflect::Member* m_struct = tD->getTreeNode(tree_0); \
		reflect::Member* m_bool = tD->getTreeNode(tree_b); \
		if (m_struct && m_bool){ \
			std::cout<<m_struct->get(this)<<" ";  \
			bool bExpanded = m_struct->expanded; \
			*(bool*)m_bool->get(this) = bExpanded; \
		} \
		std::cout<<sub_classes.size()<<":\n";\
		for (int i = 0; i < sub_classes.size(); i++) { \
			std::vector<int> tree = tree_0; \
			tree.push_back(tree_type::vec_pos); \
			tree.push_back(i); \
			sub_classes[i].read_expanded(tD, tree, tab+1); \
		} \
	}*/


#define TREE_STRUCT_ADD_FORMWIDGET_HEADER() \
	int m_i = tree_0.size() > 0 ? tree_0[tree_0.size() - 1] : 0; \
	if (type_struct != NULL && hasSubs){ \
		ExButton_FormField* ff = new ExButton_FormField(); \
		ff->tree_pos = tree_0; \
		ff->text = type_struct->members[m_i].expanded ? "-" : "+"; \
		ff->tab = (tab == -1 ? 0 : tab); \
		ff->setVisible(bVisible); \
		win->addEditField(ff); \
	}\


#define TREE_STRUCT_ADD_FORMWIDGET_FOOTER() \
	if (type_struct == NULL && hasSubs) \
		{ \
			std::vector<int> tree_1 = tree_0; \
			tree_1.push_back(vec_pos); \
			reflect::TypeDescriptor_Struct* td_struct = (reflect::TypeDescriptor_Struct*)members[vec_pos].type; \
			for (int j = 0; j < td_struct->members.size(); j++) { \
				std::vector<int> tree = tree_1; \
				tree.push_back(j); \
				td_struct->members[j].type->addFormWidget(win, td_struct, tree, members[vec_pos].offset, bVisible, bEditable, tab + 1); \
			} \
		} \
	if (type_struct != NULL && hasSubs) { \
		{ \
			std::vector<int> tree_1 = tree_0; \
			tree_1.push_back(vec_pos); \
			reflect::TypeDescriptor_Struct* td_struct = (reflect::TypeDescriptor_Struct*)members[vec_pos].type; \
			size_t new_offset = type_struct->members[m_i].offset + offset_base + members[vec_pos].offset; \
			for (int j = 0; j < td_struct->members.size(); j++) { \
				std::vector<int> tree = tree_1; \
				tree.push_back(j); \
				td_struct->members[j].type->addFormWidget(win, td_struct, tree, new_offset, type_struct->members[m_i].expanded && bVisible, bEditable, tab + 1); \
			} \
		} \
	}


// The reason these structs are declared here is to make them available
// to the template instantiator (GUI_tools.h/ GUI_tools.cpp)
//


// file_open.h
//
#endif