#pragma once

#include <string>
#include <map>
#include <memory>

template<class T>
class RessourceCache {
public:
    std::shared_ptr<T> get(const std::string& name);
	
protected:
	virtual std::shared_ptr<T> load(const std::string& name) const = 0;
	
private:
	std::map<std::string, std::weak_ptr<T>> mCache;
};

template<class T>
std::shared_ptr<T> RessourceCache<T>::get(const std::string& name) {
    std::shared_ptr<T> ressource;
    {
        auto it = mCache.find(name);
        if (mCache.end() != it)
        {
            std::weak_ptr<T> ressource_obs = it->second;
            ressource = ressource_obs.lock();
            if (!ressource)
            {
                mCache.erase(it);
            }
        }
    }
	if(!ressource)
	{
		ressource = load(name);
        std::weak_ptr<T> ressource_obs = ressource;
        mCache.insert(std::pair<std::string, std::weak_ptr<T>>(name, ressource_obs));
	}
	return ressource;
}
