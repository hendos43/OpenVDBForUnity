#include "pch.h"
#include "oiInternal.h"
#include "oiContext.h"

static std::wstring L(const std::string& s)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(s);
}

static std::string S(const std::wstring& w)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(w);
}

static std::string NormalizePath(const char *in_path)
{
    std::wstring path;

    if (in_path != nullptr) {
        path = L(in_path);

#ifdef _WIN32
        size_t n = path.length();
        for (size_t i = 0; i < n; ++i) {
            auto c = path[i];
            if (c == L'\\') {
                path[i] = L'/';
            }
            else if (c >= L'A' && c <= L'Z') {
                path[i] = L'a' + (c - L'A');
            }
        }
#endif
    }

    return S(path);
}

oiContextManager oiContextManager::s_instance;

oiContext* oiContextManager::getContext(int uid)
{
    auto it = s_instance.m_contexts.find(uid);
    if (it != s_instance.m_contexts.end()) {
        DebugLog("Using already created context for gameObject with ID %d", uid);
        return it->second.get();
    }

    auto ctx = new oiContext(uid);
    s_instance.m_contexts[uid].reset(ctx);
    DebugLog("Register context for gameObject with ID %d", uid);
    return ctx;
}

void oiContextManager::destroyContext(int uid)
{
    auto it = s_instance.m_contexts.find(uid);
    if (it != s_instance.m_contexts.end()) {
        DebugLog("Unregister context for gameObject with ID %d", uid);
        s_instance.m_contexts.erase(it);
    }
}

oiContextManager::~oiContextManager()
{
    if (m_contexts.size()) {
        DebugWarning("%lu remaining context(s) registered", m_contexts.size());
    }
    m_contexts.clear();
}


oiContext::oiContext(int uid)
    : m_uid(uid)
{
}

oiContext::~oiContext()
{
}

const std::string& oiContext::getPath() const
{
    return m_path;
}

int oiContext::getUid() const
{
    return m_uid;
}


bool oiContext::load(const char *in_path)
{
    auto path = NormalizePath(in_path);
    auto wpath = L(in_path);

    DebugLogW(L"oiContext::load: '%s'", wpath.c_str());
    if (path == m_path && m_archive)
    {
        DebugLog("oiContext::load: Context already loaded for gameObject with id %d", m_uid);
        return true;
    }

    if (path.empty())
    {
        DebugLog("oiContext::load: path is empty", m_uid);
        return false;
    }

    m_path = path;

    openvdb::io::File file(m_path);

    try
    {
        if(!file.open())
        {
            return false;
        }
    }
    catch(openvdb::IoError error)
    {
        auto message = L(error.what());
        DebugLogW(L"Failed to open archive: %s", message.c_str());
    }
    m_archive = file.copy().get();
    file.close();

    return true;
}

oiObject* oiContext::getObject() const
{
    //return m_top_node.get();
}