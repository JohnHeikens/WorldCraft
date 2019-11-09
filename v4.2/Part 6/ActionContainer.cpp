#include "ActionContainer.h"
constexpr int MAX_CONTAINERS = 0x1000;
//global variables
ActionContainer* ChunkActionContainers[MAX_CONTAINERS];
int ActionContainerCount = 0;


Action::Action(BlockIndex index, block b)
{
	this->b = b;
	this->index = index;
}

Action::~Action()
{
}

ActionContainer::ActionContainer(int posX, int posY, Action* FirstAction)
{
	this->posX = posX;
	this->posY = posY;
	actions = DBG_NEW std::list<Action*>;
	actions->push_front(FirstAction);
	ChunkActionContainers[ActionContainerCount++] = this;
}
ActionContainer::~ActionContainer()
{
	for (Action* a : (*this->actions)) 
	{
		delete a;
	}
}
ActionContainer::ActionContainer(int posX, int posY)
{
	this->posX = posX;
	this->posY = posY;
	actions = DBG_NEW std::list<Action*>;
	ChunkActionContainers[ActionContainerCount++] = this;
}
ActionContainer* GetContainer(int posX, int posY) {
	for (int i = 0; i < ActionContainerCount; i++) {
		ActionContainer* container = ChunkActionContainers[i];
		if (container->posX == posX && container->posY == posY) {
			return container;
		}
	}
	return NULL;
}
void AddAction(int PosX, int PosY, BlockIndex index, block block)
{
	ActionContainer* container = GetContainer(PosX, PosY);
	if (container == NULL) {
		container = DBG_NEW ActionContainer(PosX, PosY, DBG_NEW Action(index, block));
	}
	else {
		std::list<Action*>* actions = container->actions;
		for (auto ActiveAction = actions->begin(); ActiveAction != actions->end(); ActiveAction++)
		{
			Action* a = *ActiveAction;
			if (a->index == index) {
				a->b = block;
				return;
			}
		}
		container->actions->push_front(DBG_NEW Action(index, block));
	}
}
int GetActionContainerCount() {
	return ActionContainerCount;
}
ActionContainer* getActionContainerPointer(int index)
{
	return ChunkActionContainers[index];
}
void ClearActionContainers() {
	for (int i = 0; i < ActionContainerCount; i++)
	{
		delete ChunkActionContainers[i];
	}
	ActionContainerCount = 0;
}