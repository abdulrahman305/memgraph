function(copy_metadata_queries_e2e_python_files FILE_NAME)
    copy_e2e_python_files(metadata_queries ${FILE_NAME})
endfunction()

copy_metadata_queries_e2e_python_files(common.py)
copy_metadata_queries_e2e_python_files(show_node_labels_info.py)
copy_metadata_queries_e2e_python_files(show_edge_types_info.py)

copy_e2e_files(metadata_queries workloads.yaml)
