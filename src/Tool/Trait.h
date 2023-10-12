// c:\prj\sysw\tinysynapticsscroll\src\Tool\Trait.h - type helpers
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_sysw { namespace SmallImapsNotifier { namespace Tool { 
namespace Trait { 
// PointerToMemberFunction
namespace ptmf { 
	template<typename T> 
	struct return_type; // primary
	template<typename R, typename C, typename... Args> 
	struct return_type< R(C::*)(Args...) > { using type = R; };
	template<typename R, typename C, typename... Args> 
	struct return_type< R(C::*)(Args...) const > { using type = R; };
	template<typename T> 
	using return_type_t = typename return_type< T >::type;
	template<typename T> 
	struct clazz; // primary
	template<typename R, typename C, typename... Args> 
	struct clazz< R(C::*)(Args...) > { using type = C; };
	template<typename R, typename C, typename... Args> 
	struct clazz< R(C::*)(Args...) const > { using type = C; };
	template<typename T> 
	using class_t = typename clazz< T >::type;
} // namespace ptmf _
// PointerToStructField
namespace ptsf {
	template<typename T> 
	struct field; // primary
	template<typename R, typename C> 
	struct field< R C::* > { using type = R; };
	template<typename StructT> 
	using field_t = typename field< StructT >::type;
	template<typename T> 
	struct ztruct; // primary
	template<typename R, typename C> 
	struct ztruct< R C::* > { using type = C; };
	template<typename T> using 
	struct_t = typename ztruct< T >::type;
} // namespace ptsf _
} // namespace Trait _
}}} // namespace prj_sysw::SmallImapsNotifier::Tool _
