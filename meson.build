project(
  'wcp',
  'c',
  version: '0.4',
  license: 'GPLv3',
  default_options: ['c_std=gnu99']
)

add_project_arguments(
  [
    '-DWCPVERSION="@0@"'.format(meson.project_version()),
  ],
  language : 'c'
)

cc = meson.get_compiler('c')

wayland_client      = dependency('wayland-client')
wayland_protos      = dependency('wayland-protocols')
wayland_scanner_dep = dependency('wayland-scanner')
wayland_scanner     = find_program(
  wayland_scanner_dep.get_pkgconfig_variable('wayland_scanner')
)
xkbcommon = dependency('xkbcommon')

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
  xkbcommon
]

wcp_inc = include_directories('src/wcp',
			      'src/zen_core',
			      'src/zen_gfx')

wcp_src = ['src/wcp/wcp.c',
	   'src/wcp/wl_connector.c',
	   'src/zen_core/zc_bm_rgba.c',
	   'src/zen_core/zc_log.c',
	   'src/zen_core/zc_memory.c',
	   'src/zen_gfx/zc_draw.c'
] + protos_src + protos_headers

executable('wcp',
	    wcp_src,
	     include_directories: [wcp_inc],
	    dependencies: wcp_deps,
	    install: true)
