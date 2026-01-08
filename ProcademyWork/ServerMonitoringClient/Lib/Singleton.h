#pragma once

//
// 싱글톤 사용 방법!
//
// 1. 클래스의 생성자, 소멸자를 private으로 생성해주세요. 
// 2. 초기화, 후처리를 Init, Release 함수를 이용해주세요.
// 3. DECLARE_SINGLETON_IN_HEADER 를 클래스 내부에 선언해주세요.
//	  주의할 점! ";" 를 뒤에 붙이지 마세요!
//	  example : DECLARE_SINGLETON_IN_HEADER(type)
//    wrong   : DECLARE_SINGLETON_IN_HEADER(type);
//
// 4. DECLARE_SINGLETON_IN_CPP 를 cpp파일에 선언해주세요.
// 5. 싱글톤을 활용할때는 SINGLETON(type)을 활용해주세요.
//
// my by 정등혁
//

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

#define DECLARE_SINGLETON_IN_CPP(type)		type type::m_instance

#define SINGLETON(type) (type::GetInstance())