#include "Table.hpp"

void Table::sort(int col_num) {
    switch(col_num) {
        case 0:
            std::sort(rows.begin(), rows.end(), DataComparator::TIME);
            break;
        case 1:
            std::sort(rows.begin(), rows.end(), DataComparator::NR_T);
            break;
        case 2:
            std::sort(rows.begin(), rows.end(), DataComparator::R_T);
            break;
        case 3:
            std::sort(rows.begin(), rows.end(), DataComparator::C_T);
            break;
        case 4:
            std::sort(rows.begin(), rows.end(), DataComparator::TUP);
            break;
        case 5:
            std::sort(rows.begin(), rows.end(), DataComparator::ID);
            break;
        case 6:
            std::sort(rows.begin(), rows.end(), DataComparator::NAME);
            break;
        default:
            std::sort(rows.begin(), rows.end(), DataComparator::TIME);
            break;
    }
}
