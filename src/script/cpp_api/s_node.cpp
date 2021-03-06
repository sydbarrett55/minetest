/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "cpp_api/s_node.h"
#include "cpp_api/s_internal.h"
#include "common/c_converter.h"
#include "common/c_content.h"
#include "nodedef.h"
#include "server.h"
#include "environment.h"


struct EnumString ScriptApiNode::es_DrawType[] =
	{
		{NDT_NORMAL, "normal"},
		{NDT_AIRLIKE, "airlike"},
		{NDT_LIQUID, "liquid"},
		{NDT_FLOWINGLIQUID, "flowingliquid"},
		{NDT_GLASSLIKE, "glasslike"},
		{NDT_GLASSLIKE_FRAMED, "glasslike_framed"},
		{NDT_ALLFACES, "allfaces"},
		{NDT_ALLFACES_OPTIONAL, "allfaces_optional"},
		{NDT_TORCHLIKE, "torchlike"},
		{NDT_SIGNLIKE, "signlike"},
		{NDT_PLANTLIKE, "plantlike"},
		{NDT_FENCELIKE, "fencelike"},
		{NDT_RAILLIKE, "raillike"},
		{NDT_NODEBOX, "nodebox"},
		{0, NULL},
	};

struct EnumString ScriptApiNode::es_ContentParamType2[] =
	{
		{CPT2_NONE, "none"},
		{CPT2_FULL, "full"},
		{CPT2_FLOWINGLIQUID, "flowingliquid"},
		{CPT2_FACEDIR, "facedir"},
		{CPT2_WALLMOUNTED, "wallmounted"},
		{CPT2_LEVELED, "leveled"},
		{0, NULL},
	};

struct EnumString ScriptApiNode::es_LiquidType[] =
	{
		{LIQUID_NONE, "none"},
		{LIQUID_FLOWING, "flowing"},
		{LIQUID_SOURCE, "source"},
		{0, NULL},
	};

struct EnumString ScriptApiNode::es_ContentParamType[] =
	{
		{CPT_NONE, "none"},
		{CPT_LIGHT, "light"},
		{0, NULL},
	};

struct EnumString ScriptApiNode::es_NodeBoxType[] =
	{
		{NODEBOX_REGULAR, "regular"},
		{NODEBOX_FIXED, "fixed"},
		{NODEBOX_WALLMOUNTED, "wallmounted"},
		{NODEBOX_LEVELED, "leveled"},
		{0, NULL},
	};

ScriptApiNode::ScriptApiNode() {
}

ScriptApiNode::~ScriptApiNode() {
}

bool ScriptApiNode::node_on_punch(v3s16 p, MapNode node,
		ServerActiveObject *puncher)
{
	SCRIPTAPI_PRECHECKHEADER

	INodeDefManager *ndef = getServer()->ndef();

	// Push callback function on stack
	if(!getItemCallback(ndef->get(node).name.c_str(), "on_punch"))
		return false;

	// Call function
	push_v3s16(L, p);
	pushnode(L, node, ndef);
	objectrefGetOrCreate(puncher);
	if(lua_pcall(L, 3, 0, 0))
		scriptError("error: %s", lua_tostring(L, -1));
	return true;
}

bool ScriptApiNode::node_on_dig(v3s16 p, MapNode node,
		ServerActiveObject *digger)
{
	SCRIPTAPI_PRECHECKHEADER

	INodeDefManager *ndef = getServer()->ndef();

	// Push callback function on stack
	if(!getItemCallback(ndef->get(node).name.c_str(), "on_dig"))
		return false;

	// Call function
	push_v3s16(L, p);
	pushnode(L, node, ndef);
	objectrefGetOrCreate(digger);
	if(lua_pcall(L, 3, 0, 0))
		scriptError("error: %s", lua_tostring(L, -1));
	return true;
}

void ScriptApiNode::node_on_construct(v3s16 p, MapNode node)
{
	SCRIPTAPI_PRECHECKHEADER

	INodeDefManager *ndef = getServer()->ndef();

	// Push callback function on stack
	if(!getItemCallback(ndef->get(node).name.c_str(), "on_construct"))
		return;

	// Call function
	push_v3s16(L, p);
	if(lua_pcall(L, 1, 0, 0))
		scriptError("error: %s", lua_tostring(L, -1));
}

void ScriptApiNode::node_on_destruct(v3s16 p, MapNode node)
{
	SCRIPTAPI_PRECHECKHEADER

	INodeDefManager *ndef = getServer()->ndef();

	// Push callback function on stack
	if(!getItemCallback(ndef->get(node).name.c_str(), "on_destruct"))
		return;

	// Call function
	push_v3s16(L, p);
	if(lua_pcall(L, 1, 0, 0))
		scriptError("error: %s", lua_tostring(L, -1));
}

void ScriptApiNode::node_after_destruct(v3s16 p, MapNode node)
{
	SCRIPTAPI_PRECHECKHEADER

	INodeDefManager *ndef = getServer()->ndef();

	// Push callback function on stack
	if(!getItemCallback(ndef->get(node).name.c_str(), "after_destruct"))
		return;

	// Call function
	push_v3s16(L, p);
	pushnode(L, node, ndef);
	if(lua_pcall(L, 2, 0, 0))
		scriptError("error: %s", lua_tostring(L, -1));
}

bool ScriptApiNode::node_on_timer(v3s16 p, MapNode node, f32 dtime)
{
	SCRIPTAPI_PRECHECKHEADER

	INodeDefManager *ndef = getServer()->ndef();

	// Push callback function on stack
	if(!getItemCallback(ndef->get(node).name.c_str(), "on_timer"))
		return false;

	// Call function
	push_v3s16(L, p);
	lua_pushnumber(L,dtime);
	if(lua_pcall(L, 2, 1, 0))
		scriptError("error: %s", lua_tostring(L, -1));
	if((bool)lua_isboolean(L,-1) && (bool)lua_toboolean(L,-1) == true)
		return true;

	return false;
}

void ScriptApiNode::node_on_receive_fields(v3s16 p,
		const std::string &formname,
		const std::map<std::string, std::string> &fields,
		ServerActiveObject *sender)
{
	SCRIPTAPI_PRECHECKHEADER

	INodeDefManager *ndef = getServer()->ndef();

	// If node doesn't exist, we don't know what callback to call
	MapNode node = getEnv()->getMap().getNodeNoEx(p);
	if(node.getContent() == CONTENT_IGNORE)
		return;

	// Push callback function on stack
	if(!getItemCallback(ndef->get(node).name.c_str(), "on_receive_fields"))
		return;

	// Call function
	// param 1
	push_v3s16(L, p);
	// param 2
	lua_pushstring(L, formname.c_str());
	// param 3
	lua_newtable(L);
	for(std::map<std::string, std::string>::const_iterator
			i = fields.begin(); i != fields.end(); i++){
		const std::string &name = i->first;
		const std::string &value = i->second;
		lua_pushstring(L, name.c_str());
		lua_pushlstring(L, value.c_str(), value.size());
		lua_settable(L, -3);
	}
	// param 4
	objectrefGetOrCreate(sender);
	if(lua_pcall(L, 4, 0, 0))
		scriptError("error: %s", lua_tostring(L, -1));
}

void ScriptApiNode::node_falling_update(v3s16 p)
{
	SCRIPTAPI_PRECHECKHEADER
	lua_getglobal(L, "nodeupdate");
	push_v3s16(L, p);
	if(lua_pcall(L, 1, 0, 0))
		scriptError("error: %s", lua_tostring(L, -1));
}

void ScriptApiNode::node_falling_update_single(v3s16 p)
{
	SCRIPTAPI_PRECHECKHEADER
	lua_getglobal(L, "nodeupdate_single");
	push_v3s16(L, p);
	if(lua_pcall(L, 1, 0, 0))
		scriptError("error: %s", lua_tostring(L, -1));
}
