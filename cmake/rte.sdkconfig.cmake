execute_process(
    COMMAND bash -c "${CC} -E -undef -P -x assembler-with-cpp -ffreestanding -o ${RTE_OUTPUT}/.config ${RTE_CONFIG_TEMPLATE}"
)

execute_process(
    COMMAND bash -c "rm -rf ${RTE_OUTPUT}/include ${RTE_OUTPUT}/lib"
)

execute_process(
    COMMAND bash -c "${RTE_SDK}/script/gen-config-h.sh ${RTE_OUTPUT}/.config    \
                    > ${RTE_OUTPUT}/include/rte_config.h"
)
