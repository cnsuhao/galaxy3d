#ifndef __EditorWindowProperty_hpp__
#define __EditorWindowProperty_hpp__

#include "EditorWindow.hpp"

class EditorWindowProperty : public EditorWindow
{
protected:
	virtual void Start()
	{
		EditorWindow::Start();

		label_title->SetText("Property");

		GetGameObject()->SetLayerRecursively(Layer::Editor);
	}
};


#endif