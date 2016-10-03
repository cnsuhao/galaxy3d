#ifndef __EditorWindowConsole_hpp__
#define __EditorWindowConsole_hpp__

#include "EditorWindow.hpp"

class EditorWindowConsole : public EditorWindow
{
protected:
	virtual void Start()
	{
		EditorWindow::Start();

		label_title->SetText("Console");

		GetGameObject()->SetLayerRecursively(Layer::Editor);
	}
};


#endif