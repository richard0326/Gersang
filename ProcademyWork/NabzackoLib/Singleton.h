#pragma once

// 클래스 너무의 GetInstance 선언부
#define DECLARE_SINGLETON_IN_HEADER(type)	\
private:									\
static type m_instance;						\
public:										\
static type* GetInstance(void)				\
{											\
	return &m_instance;						\
}											\
type(const type&);							\
type& operator= (const type&);				\
private:									

// cpp 파일의 정의부
#define DECLARE_SINGLETON_IN_CPP(type)		type type::m_instance

// GetInstance 호출
#define SINGLETON(type) (type::GetInstance())