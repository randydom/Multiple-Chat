#ifndef _PROMETA_SINGLETON_
#define _PROMETA_SINGLETON_

template <typename T>
class Singleton {
private:
	static T* m_pInstance;
protected:
	Singleton() { }
	virtual ~Singleton() { }

public:
	static T* getInstance() {
		if (m_pInstance == nullptr)
			m_pInstance = new T;

		return m_pInstance;
	}

	static void destroyInstance() {
		if (m_pInstance != nullptr) {
			delete m_pInstance;
			m_pInstance = nullptr;
		}
	}
};

template <typename T> T* Singleton<T>::m_pInstance = nullptr;

#endif