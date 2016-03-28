#ifndef __LabelNode_h__
#define __LabelNode_h__

#include "Component.h"
#include "Label.h"
#include "Vector4.h"

namespace Galaxy3D
{
	class LabelBatchRenderer;

	class LabelNode : public Component
	{
		DECLARE_COM_CLASS(LabelNode, Component);

	public:
		static bool Less(const std::shared_ptr<LabelNode> &s1, const std::shared_ptr<LabelNode> &s2)
		{
			return s1->m_sorting_order < s2->m_sorting_order;
		}
		LabelNode():
			m_color(1, 1, 1, 1),
			m_sorting_order(0),
            m_dirty(true)
		{}
		void SetLabel(const std::shared_ptr<Label> &sprite);
		std::shared_ptr<Label> GetLabel() const {return m_label;}
		void SetColor(const Color &color);
		Color GetColor() const {return m_color;}
		void SetSortingOrder(int order);
        int GetSortingOrder() const {return m_sorting_order;}
        void SetAnchor(const Vector4 &anchor);
        void SetBatch(const std::weak_ptr<LabelBatchRenderer> &batch) {m_batch = batch;}
        std::weak_ptr<LabelBatchRenderer> GetBatch() const {return m_batch;}
        bool IsDirty() const {return m_dirty;}
        void SetDirty(bool dirty) {m_dirty = dirty;}

    protected:
        virtual void Start();
        virtual void OnTranformChanged();
        virtual void OnEnable() {m_dirty = true;}
        virtual void OnDisable() {m_dirty = true;}

	private:
		std::shared_ptr<Label> m_label;
		Color m_color;
        std::shared_ptr<Vector4> m_anchor;
		int m_sorting_order;
        std::weak_ptr<LabelBatchRenderer> m_batch;
        bool m_dirty;
	};
}

#endif