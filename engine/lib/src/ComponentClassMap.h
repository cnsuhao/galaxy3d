#ifndef __ComponentClassMap_h__
#define __ComponentClassMap_h__

#define DECLARE_COM_BASE(CBase) \
    public: \
        typedef CBase *(*ClassGen)(); \
        static CBase *Create(const std::string &class_name) \
        { \
            auto iter = m_class_map.find(class_name); \
            if(m_class_map.end() != iter) \
            { \
                return iter->second(); \
            } \
            return NULL; \
        } \
        virtual std::string GetTypeName() {return #CBase;} \
    protected: \
        static void Register(const std::string &class_name, ClassGen class_gen) \
        { \
            m_class_map.insert(std::unordered_map<std::string, ClassGen>::value_type(class_name, class_gen)); \
        } \
    private: \
        static std::unordered_map<std::string, ClassGen> m_class_map;

#define DEFINE_COM_BASE(CBase)  \
    std::unordered_map<std::string, CBase::ClassGen> CBase::m_class_map;

#define DECLARE_COM_CLASS(CDerived, CBase) \
    public: \
        virtual std::string GetTypeName() {return #CDerived;} \
    private: \
        struct CDerived##Register \
        { \
            CDerived##Register() \
            { \
                static bool bRegistered = false; \
                if(!bRegistered) \
                { \
                    CBase::Register(#CDerived, CDerived::Create); \
                    bRegistered = true; \
                } \
            } \
        }; \
        static CDerived##Register m_##CDerived##Register; \
        static CBase *Create() \
        { \
            return new CDerived(); \
        }

#define DEFINE_COM_CLASS(CDerived) \
    CDerived::CDerived##Register CDerived::m_##CDerived##Register; \

#endif