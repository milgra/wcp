project(
  'wcp',
  'c',
  version: '0.77b',
  license: 'MIT',
  default_options: ['c_std=gnu99',
  		    'warning_level=3']
)

cc = meson.get_compiler('c')

egl = dependency('egl')
png = dependency('libpng')
math = cc.find_library('m')
wegl = dependency('wayland-egl')
glesv2 = dependency('glesv2')
freetype = dependency('freetype2')
xkbcommon = dependency('xkbcommon')


wayland_client      = dependency('wayland-client')
wayland_cursor      = dependency('wayland-cursor')
wayland_protos      = dependency('wayland-protocols')
wayland_scanner_dep = dependency('wayland-scanner')
wayland_scanner     = find_program(
  wayland_scanner_dep.get_pkgconfig_variable('wayland_scanner')
)

wl_protocol_dir = wayland_protos.get_pkgconfig_variable('pkgdatadir')

protocols = [
  [wl_protocol_dir, 'stable/xdg-shell/xdg-shell.xml'],
  [wl_protocol_dir, 'unstable/xdg-output/xdg-output-unstable-v1.xml'],
  [wl_protocol_dir, 'unstable/pointer-gestures/pointer-gestures-unstable-v1.xml'],
  'wlr-layer-shell-unstable-v1.xml'
]

protos_src = []
protos_headers = []

foreach p : protocols
  xml = join_paths(p)
  protos_src += custom_target(
    xml.underscorify() + '_client_c',
    input: xml,
    output: '@BASENAME@-protocol.c',
    command: [wayland_scanner, 'public-code', '@INPUT@', '@OUTPUT@'],
  )
  protos_headers += custom_target(
    xml.underscorify() + '_client_h',
    input: xml,
    output: '@BASENAME@-client-protocol.h',
    command: [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
  )
endforeach


wcp_dependencies = [wayland_client,
		    wayland_cursor,
		    png,
		    freetype,
		    math,
		    glesv2,
		    egl,
		    wegl,
		    xkbcommon]

wcp_version = '"@0@"'.format(meson.project_version())
pkg_datadir = join_paths(get_option('prefix'), get_option('datadir')) / 'wcp'
add_project_arguments('-DPKG_DATADIR="' + pkg_datadir + '"',
		      '-DWCP_VERSION=@0@'.format(wcp_version),
		      '-Wno-unused-parameter',
		      language: 'c')
add_project_arguments('-Wno-unused-but-set-variable', language: 'c')

if get_option('buildtype') == 'debug'
    add_project_arguments('-DDEBUG',language: 'c')
endif

wcp_inc = include_directories(
	'src/mt_math',
	'src/mt_core',
	'src/kinetic_ui',
	'src/kinetic_ui/egl',
	'src/kinetic_ui/handler',
	'src/kinetic_ui/texture',
	'src/wcp',
	'src/mt_core_ext')

if build_machine.system() == 'freebsd'
   epoll = cc.find_library('epoll-shim')
   sov_dependencies += epoll
endif

com_sources = ['src/wcp/ui.c',
	       'src/wcp/config.c',
	       'src/wcp/kvlist.c',
	       'src/wcp/ku_bitmap_ext.c',
	       
	       'src/mt_core_ext/mt_string_ext.c',

	       'src/mt_core/mt_string.c',
	       'src/mt_core/mt_path.c',
	       'src/mt_core/mt_log.c',
	       'src/mt_core/mt_map.c',
	       'src/mt_core/mt_memory.c',
	       'src/mt_core/mt_time.c',
	       'src/mt_core/mt_vector.c',
	       'src/mt_core/mt_wrapper.c',

	       'src/mt_math/mt_matrix_4d.c',

	       'src/kinetic_ui/ku_event.c',
	       'src/kinetic_ui/ku_html.c',
	       'src/kinetic_ui/ku_css.c',
	       'src/kinetic_ui/egl/ku_gl.c',
	       'src/kinetic_ui/ku_rect.c',
	       'src/kinetic_ui/ku_bitmap.c',
	       'src/kinetic_ui/texture/tg_css.c',
	       'src/kinetic_ui/texture/tg_text.c',
	       'src/kinetic_ui/handler/vh_anim.c',
	       'src/kinetic_ui/handler/vh_button.c',
	       'src/kinetic_ui/handler/vh_drag.c',
	       'src/kinetic_ui/handler/vh_key.c',
	       'src/kinetic_ui/handler/vh_slider.c',
	       'src/kinetic_ui/handler/vh_textinput.c',
	       'src/kinetic_ui/handler/vh_touch.c',
	       'src/kinetic_ui/ku_view.c',
	       'src/kinetic_ui/egl/ku_gl_atlas.c',
	       'src/kinetic_ui/egl/ku_gl_floatbuffer.c',
	       'src/kinetic_ui/egl/ku_gl_shader.c',
	       'src/kinetic_ui/ku_gen_html.c',
	       'src/kinetic_ui/ku_gen_css.c',
	       'src/kinetic_ui/ku_gen_type.c',
	       'src/kinetic_ui/ku_renderer_egl.c',
	       'src/kinetic_ui/ku_renderer_soft.c',
	       'src/kinetic_ui/ku_fontconfig.c',
	       'src/kinetic_ui/ku_connector_wayland.c',
	       'src/kinetic_ui/ku_window.c',
	       'src/kinetic_ui/ku_png.c',
	       'src/kinetic_ui/ku_draw.c',
	       'src/kinetic_ui/ku_text.c',
	       'src/kinetic_ui/ku_gen_textstyle.c'] + protos_src + protos_headers

pkg_datadir = join_paths(get_option('prefix'), get_option('datadir')) / 'wcp'

install_data('res/html/main.html', install_dir : pkg_datadir / 'html' )
install_data('res/html/main.css', install_dir : pkg_datadir / 'html' )
install_data('res/img/bluetooth.png', install_dir : pkg_datadir / 'img' )
install_data('res/img/brightness.png', install_dir : pkg_datadir / 'img' )
install_data('res/img/display.png', install_dir : pkg_datadir / 'img' )
install_data('res/img/exit.png', install_dir : pkg_datadir / 'img' )
install_data('res/img/lock.png', install_dir : pkg_datadir / 'img' )
install_data('res/img/reboot.png', install_dir : pkg_datadir / 'img' )
install_data('res/img/shutdown.png', install_dir : pkg_datadir / 'img' )
install_data('res/img/speaker.png', install_dir : pkg_datadir / 'img' )
install_data('res/img/suspend.png', install_dir : pkg_datadir / 'img' )
install_data('res/img/wifi.png', install_dir : pkg_datadir / 'img' )
install_data('res/script/bluetooth-label.sh', install_dir : pkg_datadir / 'script' )
install_data('res/script/brightness.sh', install_dir : pkg_datadir / 'script' )
install_data('res/script/lock.sh', install_dir : pkg_datadir / 'script' )
install_data('res/script/logout.sh', install_dir : pkg_datadir / 'script' )
install_data('res/script/open-audio.sh', install_dir : pkg_datadir / 'script' )
install_data('res/script/open-bluetooth.sh', install_dir : pkg_datadir / 'script' )
install_data('res/script/open-displays.sh', install_dir : pkg_datadir / 'script' )
install_data('res/script/open-wifi.sh', install_dir : pkg_datadir / 'script' )
install_data('res/script/reboot.sh', install_dir : pkg_datadir / 'script' )
install_data('res/script/shutdown.sh', install_dir : pkg_datadir / 'script' )
install_data('res/script/suspend.sh', install_dir : pkg_datadir / 'script' )
install_data('res/script/volume.sh', install_dir : pkg_datadir / 'script' )
install_data('res/script/wifi-label.sh', install_dir : pkg_datadir / 'script' )

wcp_sources = [com_sources, 'src/wcp/wcp.c']

executable(
  'wcp',
  wcp_sources,
  include_directories: [wcp_inc],
  dependencies: wcp_dependencies,
  install: true,
)

draw_sources = [com_sources, 'src/wcp/draw.c']

executable(
  'draw',
  draw_sources,
  include_directories: [wcp_inc],
  dependencies: wcp_dependencies,
  install: false,
)

mt_core_test = executable(
	   'mt_core_test',
	   ['src/mt_core/mt_log.c',
	    'src/mt_core/mt_memory.c',
	    'src/mt_core/mt_vector.c',
	    'src/mt_core/mt_map.c',
	    'src/mt_core/mt_string.c',
	    'src/mt_core/mt_channel.c',
	    'src/mt_core/mt_path.c',
	    'src/mt_core_test/mt_core_test.c',
	    'src/mt_core_test/mt_memory_test.c',
	    'src/mt_core_test/mt_vector_test.c',
	    'src/mt_core_test/mt_map_test.c',
	    'src/mt_core_test/mt_string_test.c',
	    'src/mt_core_test/mt_channel_test.c',
	    'src/mt_core_test/mt_path_test.c'
	    ],
	    include_directories: ['src/mt_core'])

test('MT_CORE_TEST', mt_core_test)

scripttests = find_program('tst/runtests.sh')

test('SCRIPTED TESTS', scripttests, timeout: 0, workdir : meson.current_source_dir(), args : [meson.current_build_dir(),meson.current_source_dir()])
