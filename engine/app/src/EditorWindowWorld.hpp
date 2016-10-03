#ifndef __EditorWindowWorld_hpp__
#define __EditorWindowWorld_hpp__

#include "EditorWindow.hpp"

class EditorWindowWorld : public EditorWindow
{
protected:
	virtual void Start()
	{
		EditorWindow::Start();

		label_title->SetText("World");

		GetGameObject()->SetLayerRecursively(Layer::Editor);
	}
};


#endif