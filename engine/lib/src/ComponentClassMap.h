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
    protected: \
        static void Register(const std::string &class_name, ClassGen class_gen) \
        { \
            m_class_map.insert(std::unordered_map<std::string, ClassGen>::value_type(class_name, class_gen)); \
        } \
        static std::unordered_map<std::string, ClassGen> m_class_map;

#endif