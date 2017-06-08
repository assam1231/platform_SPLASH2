/*
Copyright (c) 2009 GreenSocs Ltd

Permission is hereby granted, free of charge, to any person  
obtaining a copy of this software and associated documentation files  
(the "Software"), to deal in the Software without restriction,  
including without limitation the rights to use, copy, modify, merge,  
publish, distribute, sublicense, and/or sell copies of the Software,  
and to permit persons to whom the Software is furnished to do so,  
subject to the following conditions:

The above copyright notice and this permission notice shall be  
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,  
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF  
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND  
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS  
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN  
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN  
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  
SOFTWARE.
*/

template <unsigned int BUSWIDTH,typename TRAITS, unsigned int N, typename BIND_BASE>
OCPIP_VERSION::infr::target_socket_base<BUSWIDTH,TRAITS,N,BIND_BASE>::target_socket_base(const char* name)
                                     : base_type(name,static_cast<typename BIND_BASE::bind_base_type*>(this))
                                     , bind_checker_type(base_type::name(), (base_type*)this)
//                                     , ext_support_type(BUSWIDTH)
                                     , base_eoe_done(false)
{
}

template <unsigned int BUSWIDTH,typename TRAITS, unsigned int N, typename BIND_BASE>
void OCPIP_VERSION::infr::target_socket_base<BUSWIDTH,TRAITS,N,BIND_BASE>::end_of_elaboration(){
  if (base_type::m_eoe_disabled) {
    return;
  }
  base_type::end_of_elaboration(); base_eoe_done=true;
  for (unsigned int i=0; i<base_type::size(); i++)
    bind_checker_type::check_binding(i);
}
  
template <unsigned int BUSWIDTH,typename TRAITS, unsigned int N, typename BIND_BASE>
void OCPIP_VERSION::infr::target_socket_base<BUSWIDTH,TRAITS,N,BIND_BASE>::bound_to(const std::string&, typename BIND_BASE::bind_base_type*, unsigned int){}

template <unsigned int BUSWIDTH,typename TRAITS, unsigned int N, typename BIND_BASE>
const std::string& OCPIP_VERSION::infr::target_socket_base<BUSWIDTH,TRAITS,N,BIND_BASE>::get_type_string(){return s_kind;}

template <unsigned int BUSWIDTH, typename TRAITS, unsigned int N, typename BIND_BASE>
const std::string OCPIP_VERSION::infr::target_socket_base<BUSWIDTH, TRAITS,N, BIND_BASE>::s_kind="basic_green_socket";
