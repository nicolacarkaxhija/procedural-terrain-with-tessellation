#pragma once
#include "../Engine/Window.h"
#include "renderObjectInterface.h"

#include <list>

class UserInterface : public renderObjectInterface
{
public:
	UserInterface(Window& w);
	~UserInterface();

	virtual void draw();
	virtual void update();

	UserInterface& subscribe(renderObjectInterface* subscriber);

private:
	// the other drawableObjects expose their setGui() methods (because he let the user handle their own attributes), so they can be subscribed to the GUI class
	// the GUI class will call the setGui() method for each subscriber
	std::list<renderObjectInterface*> subscribers;

};

