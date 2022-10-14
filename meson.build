project(
  'wcp',
  'c',
  version: '0.5',
  license: 'GPLv3',
  default_options: ['c_std=gnu99']
)

pkg_datadir = join_paths(get_option('prefix'), get_option('datadir')) / 'wcp'

add_project_arguments(
  [
    '-DPKG_DATADIR="' + pkg_datadir + '"',
    '-DWCP_VERSION="@0@"'.format(meson.project_version()),
  ],
  language : 'c'
)

if get_option('buildtype') == 'debug'
    add_project_arguments('-DDEBUG',language: 'c')
endif

cc = meson.get_compiler('c')

math = cc.find_library('m',static:false)
png = cc.find_library('png',static:false)
freetype = cc.find_library('freetype',static:false)
wayland_client      = dependency('wayland-client')
wayland_protos      = dependency('wayland-protocols')
wayland_scanner_dep = dependency('wayland-scanner')
wayland_scanner     = find_program(
  wayland_scanner_dep.get_pkgconfig_variable('wayland_scanner')
)

wl_protocol_dir = wayland_protos.get_pkgconfig_variable('pkgdatadir')

protocols = [
  [wl_protocol_dir, 'stable/xdg-shell/xdg-shell.xml'],
  [wl_protocol_dir, 'unstable/xdg-output/xdg-output-unstable-v1.xml'],
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

wcp_deps = [
  wayland_client,
  freetype,
  png,
  math
]

wcp_inc = include_directories('src/wcp',
			      'src/wcp/ui',
			      'src/wcp/utils',
			      'src/zen_core',
			      'src/zen_math',
			      'src/zen_ui',
			      'src/zen_ui/view',
			      'src/zen_ui/html',
			      'src/zen_gfx',
			      '/usr/include/freetype2')

wcp_src = [    'src/wcp/wcp.c',
	       'src/wcp/wl_connector.c',
	       'src/wcp/wm_event.c',
	       'src/wcp/config.c',

	       'src/wcp/ui/ui.c',

	       'src/wcp/utils/bm_rgba_util.c',
	       'src/wcp/utils/cstr_util.c',
	       'src/wcp/utils/str_util.c',
	       'src/wcp/utils/map_util.c',

	       'src/zen_math/zc_mat3.c',
	       'src/zen_math/zc_mat4.c',
	       'src/zen_math/zc_vec2.c',
	       'src/zen_math/zc_vec3.c',
	       'src/zen_math/zc_vec4.c',
	       'src/zen_math/zc_util2.c',
	       'src/zen_math/zc_util3.c',

	       'src/zen_core/zc_bm_rgba.c',
	       'src/zen_core/zc_callback.c',
	       'src/zen_core/zc_channel.c',
	       'src/zen_core/zc_cstring.c',
	       'src/zen_core/zc_path.c',
	       'src/zen_core/zc_log.c',
	       'src/zen_core/zc_map.c',
	       'src/zen_core/zc_memory.c',
	       'src/zen_core/zc_number.c',
	       'src/zen_core/zc_string.c',
	       'src/zen_core/zc_time.c',
	       'src/zen_core/zc_vector.c',
	       'src/zen_core/zc_wrapper.c',

	       'src/zen_gfx/zc_draw.c',
	       'src/zen_gfx/zc_text.c',

	       'src/zen_ui/html/html.c',
	       'src/zen_ui/html/css.c',
	       'src/zen_ui/view/tg_css.c',
	       'src/zen_ui/view/tg_knob.c',
	       'src/zen_ui/view/tg_picker.c',
	       'src/zen_ui/view/tg_text.c',
	       'src/zen_ui/view/tg_scaledimg.c',
	       'src/zen_ui/view/vh_anim.c',
	       'src/zen_ui/view/vh_button.c',
	       'src/zen_ui/view/vh_drag.c',
	       'src/zen_ui/view/vh_key.c',
	       'src/zen_ui/view/vh_knob.c',
	       'src/zen_ui/view/vh_slider.c',
	       'src/zen_ui/view/vh_picker.c',
	       'src/zen_ui/view/vh_roll.c',
	       'src/zen_ui/view/vh_textinput.c',
	       'src/zen_ui/view/vh_touch.c',
	       'src/zen_ui/view/vh_cv_body.c',
	       'src/zen_ui/view/vh_cv_scrl.c',
	       'src/zen_ui/view/vh_cv_evnt.c',
	       'src/zen_ui/view/vh_tbl_body.c',
	       'src/zen_ui/view/vh_tbl_evnt.c',
	       'src/zen_ui/view/vh_tbl_scrl.c',
	       'src/zen_ui/view/vh_tbl_head.c',
	       'src/zen_ui/view/view.c',
	       'src/zen_ui/view/viewgen_html.c',
	       'src/zen_ui/view/viewgen_css.c',
	       'src/zen_ui/view/viewgen_type.c',
	       'src/zen_ui/view/view_layout.c',
	       'src/zen_ui/view/views.c',
	       'src/zen_ui/fontconfig.c',
	       'src/zen_ui/ui_generator.c',
	       'src/zen_ui/ui_manager.c',
	       'src/zen_ui/ui_texmap.c',
	       'src/zen_ui/ui_table.c',
	       'src/zen_ui/ui_util.c'] + protos_src + protos_headers

executable('wcp',
	    wcp_src,
	     include_directories: [wcp_inc],
	    dependencies: wcp_deps,
	    install: true)

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
