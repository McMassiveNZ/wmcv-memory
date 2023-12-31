include_guard()

function( target_precompile_headers _target _visibility _header _source )
	if( MSVC )
		set_target_properties( ${_target} PROPERTIES COMPILE_FLAGS "/Yu${_header}" )
		set_source_files_properties( ${_source} PROPERTIES COMPILE_FLAGS "/Yc${_header}" )
	else()
		_target_precompile_headers(${_target} ${_visibility} ${_header} ${_source})
	endif()
endfunction()