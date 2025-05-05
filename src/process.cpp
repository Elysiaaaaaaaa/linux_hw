#include "process.h"


process::process(int switch_time,int num_mailboxes, int mem_size, int proj_id, const char *pathname) {
    this->switch_time = switch_time;
    this->pathname = pathname;
    this->proj_id = proj_id;
    // ��ȡ Tunnel ����Ĺ����ڴ� key
    key_t shm_key_tunnel = ftok(pathname, proj_id + PROJ_SHM_TUNNEL_OFFSET);
    int shmid_tunnel = shmget(shm_key_tunnel, sizeof(Tunnel), IPC_CREAT | 0666);
    if (shmid_tunnel == -1) {
        Logger::log(LogLevel::WARN,"shmget for tunnel failed");
        exit(EXIT_FAILURE);
    }

    // �����ڴ� attach
    void *shmaddr_tunnel = shmat(shmid_tunnel, nullptr, 0);
    if (shmaddr_tunnel == (void *)-1) {
        perror("shmat for tunnel failed");
        exit(EXIT_FAILURE);
    }

    // ʹ�� placement new ���� Tunnel ʵ���ڹ����ڴ���
    tunnel = new (shmaddr_tunnel) Tunnel(proj_id, pathname);

    tunnel->current_direction_ = Direction::Eastbound;

    Signal(tunnel->direction_changed_,0);

    // ��ȡ mailbox ����Ĺ����ڴ� key
    key_t shm_key_mailbox = ftok(pathname, proj_id + PROJ_SHM_MAILBOX_OFFSET);
    int shmid_mailbox = shmget(shm_key_mailbox, sizeof(mailbox), IPC_CREAT | 0666);
    if (shmid_mailbox == -1) {
        perror("shmget for mailbox failed");
        exit(EXIT_FAILURE);
    }

    // �����ڴ� attach
    void *shmaddr_mailbox = shmat(shmid_mailbox, nullptr, 0);
    if (shmaddr_mailbox == (void *)-1) {
        perror("shmat for mailbox failed");
        exit(EXIT_FAILURE);
    }

    // ʹ�� placement new ���� mailbox ʵ���ڹ����ڴ���
    mail_box = new (shmaddr_mailbox) mailbox(num_mailboxes, mem_size, proj_id, pathname);

    // ��ʼ���������飨������Ҳ�ɣ�
    cars.reserve(total_number_of_cars);
}




process::~process() {
    // �ͷ� Tunnel ��صĹ����ڴ�
    if (tunnel != nullptr) {
        // ������ Tunnel ����
        tunnel->~Tunnel();

        // Ȼ���ͷŹ����ڴ�
        key_t shm_key_tunnel = ftok(pathname, proj_id + PROJ_SHM_TUNNEL_OFFSET);
        int shmid_tunnel = shmget(shm_key_tunnel, sizeof(Tunnel), 0666);
        if (shmid_tunnel != -1) {
            // ɾ�������ڴ�
            if (shmctl(shmid_tunnel, IPC_RMID, nullptr) == -1) {
                perror("shmctl for tunnel failed");
            }
        }
    }

    // �ͷ� mailbox ��صĹ����ڴ�
    if (mail_box != nullptr) {
        // ������ mailbox ����
        mail_box->~mailbox();

        // Ȼ���ͷŹ����ڴ�
        key_t shm_key_mailbox = ftok(pathname, proj_id + PROJ_SHM_MAILBOX_OFFSET);
        int shmid_mailbox = shmget(shm_key_mailbox, sizeof(mailbox), 0666);
        if (shmid_mailbox != -1) {
            // ɾ�������ڴ�
            if (shmctl(shmid_mailbox, IPC_RMID, nullptr) == -1) {
                perror("shmctl for mailbox failed");
            }
        }
    }

    // ����������
    cars.clear();
}


void process::init_car(txt_reader& reader) {
    int idx;
    int direct;
    reader.buf >> idx >> direct;
    cars.emplace_back(idx, static_cast<Direction>(direct), reader);
}


void process::enter(Car *car) {
    if(!use_rg) {
//        �޺��̵�
        Wait(tunnel->mutex_, 0); // ��ȡ�ź���
        while (true) {
            if (tunnel->car_count_ == 0) {
                // ���û�г������÷��򲢽���
                if(tunnel->current_direction_ != car->direction_){
                    switchDirection();
                }
                tunnel->car_count_ += 1;
                if(car->direction_ == Direction::Eastbound){
                    Wait(tunnel->block_,0);
                }else{
                    Wait(tunnel->block_,1);
                }
                car->start_time = std::chrono::high_resolution_clock::now();
                car->state = State::INNER;
                Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                            " entering tunnel in direction " +
                                            std::to_string(static_cast<int>(car->direction_)) +
                                            " (empty tunnel).");
                break;
            } else if (tunnel->current_direction_ == car->direction_ &&
                       tunnel->car_count_ < maximum_number_of_cars_in_tunnel) {
                // ͬһ������δ�ﵽ�������������
                (tunnel->car_count_)++;
                if(car->direction_ == Direction::Eastbound){
                    Wait(tunnel->block_,0);
                }else{
                    Wait(tunnel->block_,1);
                }
                car->start_time = std::chrono::high_resolution_clock::now();
                car->state = State::INNER;
                Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                            " entering tunnel in direction " +
                                            std::to_string(static_cast<int>(car->direction_)) +
                                            " (same direction, space available).");
                break;
            } else if (tunnel->current_direction_ != car->direction_) {
                // ����ͬ���ȴ�
                if(!car->out) {
                    car->out = true;
                    Logger::log(LogLevel::WARN, "Car " + std::to_string(car->car_id) +
                                                " waiting due to opposite direction (direction " +
                                                std::to_string(static_cast<int>(car->direction_)) +
                                                "), tunnel occupied by direction " +
                                                std::to_string(static_cast<int>(tunnel->current_direction_)) + ".");
                }
                Signal(tunnel->mutex_, 0); // �ͷ��ź���
                if(car->direction_ == Direction::Eastbound){
                    Wait(tunnel->direction_changed_,0);
                    Signal(tunnel->direction_changed_,0);
                }else{
                    Wait(tunnel->direction_changed_,1);
                    Signal(tunnel->direction_changed_,1);
                }
                Wait(tunnel->mutex_, 0); // �ٴλ�ȡ�ź���
            } else {
                // ����������ȴ�
                if(!car->out) {
                    car->out = true;
                    Logger::log(LogLevel::WARN, "Car " + std::to_string(car->car_id) +
                                                " waiting because tunnel full (direction " +
                                                std::to_string(static_cast<int>(car->direction_)) +
                                                ", cars in tunnel: " + std::to_string(tunnel->car_count_) + ").");
                }
                Signal(tunnel->mutex_, 0); // �ͷ��ź���
                if(car->direction_ == Direction::Eastbound){
                    Wait(tunnel->block_,0);
                    Signal(tunnel->block_,0);
                }else{
                    Wait(tunnel->block_,1);
                    Signal(tunnel->block_,1);
                }
                Wait(tunnel->mutex_, 0); // �ٴλ�ȡ�ź���
            }

        }
        Signal(tunnel->mutex_, 0); // �ͷ��ź���
    }
    else{
    //    �к��̵�
        Wait(tunnel->mutex_, 0); // ��ȡ�ź���
        while (true) {
            if (isGreenLight(car->direction_) && tunnel->car_count_ < maximum_number_of_cars_in_tunnel) {
                // ͬһ������δ�ﵽ����������������Խ���
                if(tunnel->car_count_==0){
                    if(car->direction_==Direction::Eastbound)
                        Wait(tunnel->zero_car_, 0);
                    else
                        Wait(tunnel->zero_car_, 1);
                }
                (tunnel->car_count_)++;
                if(car->direction_ == Direction::Eastbound){
                    Wait(tunnel->block_,0);
                }else{
                    Wait(tunnel->block_,1);
                }
                car->start_time = std::chrono::high_resolution_clock::now();
                car->state = State::INNER;
                Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) +
                                            " entering tunnel in direction " +
                                            std::to_string(static_cast<int>(car->direction_)) +
                                            " (same direction, space available).");
                break;
            } else if (tunnel->current_direction_ != car->direction_) {
    //            ����ͬ
                if(!car->out) {
//                    car->out = true;
                    Logger::log(LogLevel::WARN, "Car " + std::to_string(car->car_id) +
                                                " waiting due to opposite direction (direction " +
                                                std::to_string(static_cast<int>(car->direction_)) +
                                                "), tunnel occupied by direction " +
                                                std::to_string(static_cast<int>(tunnel->current_direction_)) + ".");
                }
                Signal(tunnel->mutex_, 0); // �ͷ��ź���
                if(car->direction_ == Direction::Eastbound) {
                    Wait(tunnel->direction_changed_, 0); // �ȴ�����仯���ź���
                    Signal(tunnel->direction_changed_, 0); // �ȴ�����仯���ź���
                    Wait(tunnel->zero_car_, 1); // �ȴ��Է���û��
                    Signal(tunnel->zero_car_, 1);

                }
                else{
                    Wait(tunnel->direction_changed_, 1); // �ȴ�����仯���ź���
                    Signal(tunnel->direction_changed_, 1); // �ȴ�����仯���ź���
                    Wait(tunnel->zero_car_, 0); // �ȴ��Է���û��
                    Signal(tunnel->zero_car_, 0);

                }
                Wait(tunnel->mutex_, 0); // �ٴλ�ȡ�ź���
            } else {
                if(!car->out) {
//                    car->out = true;
                    Logger::log(LogLevel::WARN, "Car " + std::to_string(car->car_id) +
                                                " waiting because tunnel full (direction " +
                                                std::to_string(static_cast<int>(car->direction_)) +
                                                ", cars in tunnel: " + std::to_string(tunnel->car_count_) + ").");
                }
                    //            ������
                Signal(tunnel->mutex_, 0); // �ͷ��ź���
                if(car->direction_ == Direction::Eastbound){
                    Wait(tunnel->block_,0);
                    Signal(tunnel->block_,0);
                }else{
                    Wait(tunnel->block_,1);
                    Signal(tunnel->block_,1);
                }
                Wait(tunnel->mutex_, 0); // �ٴλ�ȡ�ź���
            }
        }
        Signal(tunnel->mutex_, 0); // �ͷ��ź���
    }
}


void process::leave(Car *car){
    Wait(tunnel->mutex_, 0); // ����

    (tunnel->car_count_)--;
    car->state = State::OUT;
    if (tunnel->car_count_ == 0&& !use_rg){
        switchDirection();
    }
    if (tunnel->car_count_ == 0 && use_rg) {
        if(car->direction_ == Direction::Eastbound){
            Signal(tunnel->zero_car_, 0); // �ȴ�����仯���ź���
        }else{
            Signal(tunnel->zero_car_, 1); // �ȴ�����仯���ź���
        }
    }
    if(car->direction_ == Direction::Eastbound){
        Signal(tunnel->block_,0);
    }else{
        Signal(tunnel->block_,1);
    }

    Signal(tunnel->mutex_, 0); // ����
    Logger::log(LogLevel::INFO, "Car " + std::to_string(car->car_id) + " Leave.");
}
bool process::isGreenLight(Direction dir) {
    return tunnel->current_direction_ == dir; // ��ǰ����Ϊ�̵�
}

void process::switchDirection() {
    if(tunnel->current_direction_ == Direction::Eastbound){
        tunnel->current_direction_ = Direction::Westbound;
        Signal(tunnel->direction_changed_,1);
        Wait(tunnel->direction_changed_,0);
    }else{
        tunnel->current_direction_ = Direction::Eastbound;
        Signal(tunnel->direction_changed_,0);
        Wait(tunnel->direction_changed_,1);
    }
    Logger::log(LogLevel::INFO, "Tunnel direction switched to " + std::string((tunnel->current_direction_ == Direction::Eastbound) ? "Eastbound" : "Westbound"));
}
void process::main_process(){
    start_time = std::chrono::high_resolution_clock::now(); // ��¼��ʼʱ��
    Logger::setBaseTime();
    Logger::initLogFile("log.txt");
    Logger::log(LogLevel::INFO, "PROCESS BEGAN");
    Logger::log(LogLevel::INFO, "Tunnel direction switched to " + std::string((tunnel->current_direction_ == Direction::Eastbound) ? "Eastbound" : "Westbound"));

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t id;
    int i = 0;
    // Ϊÿ��������һ���ӽ���
    for (; i < total_number_of_cars; ++i) {
        id = Fork();
        if (id == 0) {
            // �ӽ���
            close(pipefd[0]); // �رն���
            break;
        }
//        sleep(1);
    }

    if (id == 0) {
        // �ӽ����߼�
        if(ex_input) {
//        ����һ��ʱ�䣬ֱ��car_id % 10 * 100�Ž������
            auto car_current_time = std::chrono::high_resolution_clock::now();
            auto car_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                    car_current_time - start_time).count();
            if (cars[i].car_id%10 * 500 - car_elapsed_time > 0) {

                usleep((cars[i].car_id%10 * 500 - car_elapsed_time) * 1000);
            }
        }

        enter(&cars[i]);
        tunnel->show();
//        sleep(1);
//        ����ڵ�ÿ���������Է��ʺ��޸�һ������ģ���������ϵͳ�Ĺ����ڴ�Σ����Կ�����һ
//        �����飬���ʲ�������������r��w��������������ڵĳ������ڽ�����󱣳����ֻ�ͨѶ����
//        ���������ֻ��źţ�����������������Ҫ���ʸù����ڴ�Ρ�
        int t = 0;
        for (const auto& op : cars[i].operations) {
            t++;
            auto current_time = std::chrono::high_resolution_clock::now();
            auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                    current_time - start_time).count();
            if(ex_input){
                elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                    current_time - cars[i].start_time).count();
            }
            if (elapsed_time > op.time) {
                if(op.isWrite){
                    Logger::log(LogLevel::WARN,
                                "Car " + std::to_string(cars[i].car_id) + " has timed out for write " + to_string(op.mailbox));
                }else{
                    Logger::log(LogLevel::WARN,
                                "Car " + std::to_string(cars[i].car_id) + " has timed out for read "+to_string(op.mailbox));
                }
            } else {
                auto remaining_time = op.time - elapsed_time;
                if (remaining_time > 10) {
                    usleep((remaining_time - 10) * 1000);
                }
                cars[i].handel++;
                Logger::log(LogLevel::INFO,
                            "Car " + std::to_string(cars[i].car_id) +" lock " + to_string(op.mailbox));
                if (op.isWrite) {
                    mail_box->writeMailbox(op.mailbox - 1, op.data, op.time, start_time);
                    mail_box->read_one(op.mailbox - 1);
                } else {
                    std::string readResult;
                    mail_box->readMailbox(op.mailbox - 1, readResult, op.time, start_time);
                    cars[i].model_str = cars[i].model_str + readResult;
                    Logger::log(LogLevel::INFO, "Car " + std::to_string(cars[i].car_id) +
                                                " Reader: " + cars[i].model_str + ".");
                }
            }
        }

        // ����Ԥ�ƽ���ʱ��
        auto end_time = cars[i].start_time + cars[i].cost_time;
        // ��ȡ��ǰʱ��
        auto current_time = std::chrono::high_resolution_clock::now();
        // ������Ҫ˯�ߵ�ʱ��
        auto sleep_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - current_time);
        // �����Ҫ˯�ߣ�����ǰʱ�仹δ��Ԥ�ƽ���ʱ�䣩
        if (sleep_duration.count() > 0) {
            // ʹ�� std::this_thread::sleep_for ����������usleep ��֧�� std::chrono ���ͣ�����ʹ�� std::this_thread::sleep_for��
            usleep(sleep_duration.count()*1000);
        }
        leave(&cars[i]);
        // �� start_time ת��Ϊʱ��������룩

        auto start_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(cars[i].start_time.time_since_epoch()).count();
        if(ex_input){
            start_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(cars[i].start_time.time_since_epoch()).count() - cars[i].car_id%10 * 500;
        }
        // �������ݵ��ܵ�
        std::string data = cars[i].model_str + "|" + std::to_string(cars[i].handel) + "|" + std::to_string(cars[i].wait_handel) + "|" + std::to_string(start_time_ms);
        size_t data_length = data.length();
        write(pipefd[1], &data_length, sizeof(data_length));
        write(pipefd[1], data.c_str(), data_length);
        close(pipefd[1]);
        //        Logger::log(LogLevel::INFO,cars[i].model_str);
        //        Signal(total_number_of_cars_tunnel, 0); // ��ɺ��ͷ��ź���
        exit(0); // �ӽ�����ɺ��˳����������ִ�и����̴���

    } else {
        // �ر�д��
        close(pipefd[1]);

        // �ȴ������ӽ����˳�
        auto last_switch_time = start_time;
        while (true) {
            auto current_time = std::chrono::high_resolution_clock::now();
            auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_switch_time).count();
            if (int(elapsed_time/this->switch_time)%2&&use_rg) {
                switchDirection();
                last_switch_time = current_time;
            }

            // ����Ƿ������ӽ��̶����˳�
            int status;
            pid_t pid = waitpid(-1, &status, WNOHANG);
            if (pid == -1) {
                // û�и����ӽ���
                break;
            }
        }

        // �ӹܵ���ȡ����
        for (int i = 0; i < total_number_of_cars; ++i) {
            size_t data_length;
            ssize_t n = read(pipefd[0], &data_length, sizeof(data_length));
            if (n == sizeof(data_length)) {
                std::string data(data_length, '\0');
                ssize_t bytes_read = 0;
                while (bytes_read < static_cast<ssize_t>(data_length)) {
                    ssize_t result = read(pipefd[0], &data[bytes_read], data_length - bytes_read);
                    if (result <= 0) {
                        break;
                    }
                    bytes_read += result;
                }
                if (bytes_read == static_cast<ssize_t>(data_length)) {
                    size_t pos1 = data.find('|');
                    size_t pos2 = data.find('|', pos1 + 1);
                    size_t pos3 = data.find('|', pos2 + 1);
                    if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos) {
                        cars[i].model_str = data.substr(0, pos1);
                        cars[i].handel = std::stoi(data.substr(pos1 + 1, pos2 - pos1 - 1));
                        cars[i].wait_handel = std::stoi(data.substr(pos2 + 1, pos3 - pos2 - 1));
                        auto start_time_ms = std::stoll(data.substr(pos3 + 1));

                        cars[i].start_time = std::chrono::high_resolution_clock::time_point(std::chrono::milliseconds(start_time_ms));
                    }
                }
            }
        }
        close(pipefd[0]);
        int should = 0, had = 0, start=0;
        for (int i = 0; i < total_number_of_cars; ++i) {
            auto start_time_elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(cars[i].start_time - start_time).count();

            Logger::log(LogLevel::INFO, "Car " + std::to_string(cars[i].car_id) +
                    " Start : " + to_string(start_time_elapsed_ms) +
                    " Should do: " + to_string(cars[i].wait_handel) +
                    " Had do: " + to_string(cars[i].handel) +
                    " Reader: " + cars[i].model_str + ".");
            should += cars[i].wait_handel;
            had += cars[i].handel;
            start += int(start_time_elapsed_ms);
//            cout<<start_time_elapsed_ms<<endl;
//            cout<<start<<endl;
        }
        mail_box->show();
        Logger::log(LogLevel::INFO,"Global Finish Rate: " + to_string(1.0*had/should));
        Logger::log(LogLevel::INFO,"Mean Start Time: " + to_string(1.0*start/total_number_of_cars));
        Logger::log(LogLevel::INFO, "PROCESS FINISH");
    }
}