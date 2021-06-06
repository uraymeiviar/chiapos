// Copyright 2018 Chia Network Inc

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//    http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <ctime>
#include <set>

#include "cxxopts.hpp"
#include "../lib/include/picosha2.hpp"
#include "plotter_disk.hpp"
#include "prover_disk.hpp"
#include "verifier.hpp"
#include "thread_pool.hpp"
#include "bls/bls.hpp"
#include <ctime>
#include <iomanip>

using std::string;
using std::vector;
using std::endl;
using std::cout;

thread_pool pool(4);
synced_stream sync_out;

void HexToBytes(const string &hex, uint8_t *result)
{
    for (uint32_t i = 0; i < hex.length(); i += 2) {
        string byteString = hex.substr(i, 2);
        uint8_t byte = (uint8_t)strtol(byteString.c_str(), NULL, 16);
        result[i / 2] = byte;
    }
}

vector<unsigned char> intToBytes(uint32_t paramInt, uint32_t numBytes)
{
    vector<unsigned char> arrayOfByte(numBytes, 0);
    for (uint32_t i = 0; paramInt > 0; i++) {
        arrayOfByte[numBytes - i - 1] = paramInt & 0xff;
        paramInt >>= 8;
    }
    return arrayOfByte;
}

string Strip0x(const string &hex)
{
    if (hex.size() > 1 && (hex.substr(0, 2) == "0x" || hex.substr(0, 2) == "0X")) {
        return hex.substr(2);
    }
    return hex;
}

string hexStr(const std::vector<uint8_t>& data)
{
     std::stringstream ss;
     ss << std::hex;

     for( int i(0) ; i < data.size(); ++i )
         ss << std::setw(2) << std::setfill('0') << (int)data[i];

     return ss.str();
}

void HelpAndQuit(cxxopts::Options options)
{
    cout << options.help({""}) << endl;
    cout << "./ProofOfSpace create" << endl;
    cout << "./ProofOfSpace prove <challenge>" << endl;
    cout << "./ProofOfSpace verify <proof> <challenge>" << endl;
    cout << "./ProofOfSpace check" << endl;
    exit(0);
}

bls::AugSchemeMPL scheme;

bls::PrivateKey derive_path(bls::PrivateKey sk, std::vector<uint32_t> list) {
	for (uint32_t index : list) {
		sk = scheme.DeriveChildSk(sk,index);
	}
	return sk;
}

bls::PrivateKey master_sk_to_local_sk(bls::PrivateKey master) {
	return derive_path(master,{12381,8444,3,0});
}

int main(int argc, char *argv[]) try {
    cxxopts::Options options(
        "ProofOfSpace", "Utility for plotting, generating and verifying proofs of space.");
    options.positional_help("(create/prove/verify/check) param1 param2 ")
        .show_positional_help();

    // Default values
    uint8_t k = 32;
    uint32_t num_buckets = 128;
    uint32_t num_stripes = 65536;
    uint8_t num_threads = 2;
	string farmer_key = "";
	string pool_key = "";
    string filename = "plot.dat";
    string tempdir = ".";
    string tempdir2 = "";
    string finaldir = ".";
    string operation = "help";
    string memo = "0102030405";
    string id = "022fb42c08c12de3a6af053880199806532e79515f94e83461612101f9412f9e";
    bool nobitfield = false;
    bool show_progress = true;
    uint32_t buffmegabytes = 4608;

//sk = AugSchemeMPL.key_gen(token_bytes(32)) // where token_bytes(32) : random bytes
 
/*
	 def _derive_path(sk: PrivateKey, path: List[int]) -> PrivateKey:
	    for index in path:
	        sk = AugSchemeMPL.derive_child_sk(sk, index)
	    return sk
*/

/*
def master_sk_to_local_sk(master: PrivateKey) -> PrivateKey:
   return _derive_path(master, [12381, 8444, 3, 0])
*/

/*
@staticmethod
    def generate_plot_public_key(local_pk: G1Element, farmer_pk: G1Element) -> G1Element:
        return local_pk + farmer_pk
*/

/*
def stream_plot_info_pk(
    pool_public_key: G1Element,
    farmer_public_key: G1Element,
    local_master_sk: PrivateKey,
):
    # There are two ways to stream plot info: with a pool public key, or with a pool contract puzzle hash.
    # This one streams the public key, into bytes
    data = bytes(pool_public_key) + bytes(farmer_public_key) + bytes(local_master_sk)
    assert len(data) == (48 + 48 + 32)
    return data
*/

/*
def calculate_plot_id_pk(
        pool_public_key: G1Element,
        plot_public_key: G1Element,
    ) -> bytes32:
        return std_hash(bytes(pool_public_key) + bytes(plot_public_key))
*/

	//plot_public_key = ProofOfSpace.generate_plot_public_key(master_sk_to_local_sk(sk).get_g1(), farmer_public_key)
	//pool_public_key = G1Element.from_bytes(bytes.fromhex(args.pool_public_key))
	//plot_id = ProofOfSpace.calculate_plot_id_pk(pool_public_key, plot_public_key)

    //"n, file", "Filename", cxxopts::value<string>(filename))(
    //"m, memo", "Memo to insert into the plot", cxxopts::value<string>(memo))(
    //"i, id", "Unique 32-byte seed for the plot", cxxopts::value<string>(id))(

    options.allow_unrecognised_options().add_options()(
        "k, size", "Plot size (default:32)", cxxopts::value<uint8_t>(k))(
        "r, threads", "Number of threads (default:2)", cxxopts::value<uint8_t>(num_threads))(
        "u, buckets", "Number of buckets (default:1288)", cxxopts::value<uint32_t>(num_buckets))(
        "s, stripes", "Size of stripes (default:65536)", cxxopts::value<uint32_t>(num_stripes))(
        "t, tempdir", "Temporary directory", cxxopts::value<string>(tempdir))(
        "2, tempdir2", "Second Temporary directory", cxxopts::value<string>(tempdir2))(
        "d, finaldir", "Final directory", cxxopts::value<string>(finaldir))(
		"f, farmer-key", "Farmer public key", cxxopts::value<string>(farmer_key))(
		"p, pool-key", "Pool public key", cxxopts::value<string>(pool_key))(
        "e, nobitfield", "Disable bitfield", cxxopts::value<bool>(nobitfield))(
        "b, buffer", "Megabytes to be used as buffer for sorting and plotting (default:4608)",cxxopts::value<uint32_t>(buffmegabytes))(
        "v, progress", "Display progress percentage during plotting",cxxopts::value<bool>(show_progress))(
        "help", "Print help");

    auto result = options.parse(argc, argv);

    if (result.count("help") || argc < 2) {
        HelpAndQuit(options);
    }
    operation = argv[1];
    std::cout << "operation: " << operation << std::endl;

    if (operation == "help") {
        HelpAndQuit(options);
    } else if (operation == "create") {
		
		cout << "creating plot..." << endl;
		std::random_device r;
		std::default_random_engine randomEngine(r());
		std::uniform_int_distribution<int> uniformDist(0, UCHAR_MAX);
		std::vector<uint8_t> sk_data(32);
		std::generate(sk_data.begin(), sk_data.end(), [&uniformDist, &randomEngine] () {
			return uniformDist(randomEngine);
		});
	
		try {
			bls::PrivateKey sk = scheme.KeyGen(bls::Bytes(sk_data));
			cout << "sk        = " << hexStr(sk.Serialize()) << endl;

			bls::G1Element local_pk = master_sk_to_local_sk(sk).GetG1Element();
			cout << "local pk  = " << hexStr(local_pk.Serialize()) << endl;

			std::vector<uint8_t> farmer_key_data(48);
			HexToBytes(farmer_key,farmer_key_data.data());
			bls::G1Element farmer_pk = bls::G1Element::FromByteVector(farmer_key_data);
			cout << "farmer pk = " << hexStr(farmer_pk.Serialize()) << endl;

			bls::G1Element plot_pk = local_pk + farmer_pk;
			cout << "plot pk   = " << hexStr(plot_pk.Serialize()) << endl;

			std::vector<uint8_t> pool_key_data(48);
			HexToBytes(pool_key,pool_key_data.data());
			bls::G1Element pool_pk = bls::G1Element::FromByteVector(pool_key_data);
			cout << "pool pk   = " << hexStr(pool_pk.Serialize()) << endl;

			std::vector<uint8_t> pool_key_bytes = pool_pk.Serialize();
			std::vector<uint8_t> plot_key_bytes = plot_pk.Serialize();
			std::vector<uint8_t> farm_key_bytes = farmer_pk.Serialize();
			std::vector<uint8_t> mstr_key_bytes = sk.Serialize();
			
			std::vector<uint8_t> plid_key_bytes;
			plid_key_bytes.insert(plid_key_bytes.end(), pool_key_bytes.begin(), pool_key_bytes.end());
			plid_key_bytes.insert(plid_key_bytes.end(), plot_key_bytes.begin(), plot_key_bytes.end());

			std::vector<uint8_t> plot_id(32);
			bls::Util::Hash256(plot_id.data(),plid_key_bytes.data(),plid_key_bytes.size());
			id = hexStr(plot_id);
			cout << "plot id   = " << id << endl;

			std::vector<uint8_t> memo_data;
			memo_data.insert(memo_data.end(), pool_key_bytes.begin(), pool_key_bytes.end());
			memo_data.insert(memo_data.end(), farm_key_bytes.begin(), farm_key_bytes.end());
			memo_data.insert(memo_data.end(), mstr_key_bytes.begin(), mstr_key_bytes.end());
			memo = hexStr(memo_data);
			cout << "memo      = " << memo << endl;

			time_t t = std::time(nullptr);
			std::tm tm = *std::localtime(&t);
			std::ostringstream oss;
			oss << std::put_time(&tm, "%Y-%m-%d-%H-%M");
			string timestr = oss.str();

			filename = "plot-k"+std::to_string((int)k)+"-"+timestr+"-"+id+".plot";
			std::filesystem::path destPath = std::filesystem::path(finaldir);
			if (std::filesystem::exists(destPath)) {
				if (std::filesystem::is_regular_file(destPath)) {
					destPath = destPath.parent_path() / filename;
				}
				else {
					destPath = destPath / filename;
				}
			}
			else {
				if (std::filesystem::create_directory(destPath)) {
					destPath = destPath / filename;
				}
				else {
					std::cerr << "unable to create directory " << destPath.string() << std::endl;
					exit(1);
				}
			}
			if(std::filesystem::exists(destPath)) {
				std::cerr << "plot file already exists " << destPath.string() << std::endl;
				exit(1);
			}

			std::filesystem::path tempPath = std::filesystem::path(tempdir);
			if (std::filesystem::exists(tempPath)) {
				if (std::filesystem::is_regular_file(tempPath)) {
					tempPath = tempPath.parent_path();
				}
			}
			else {
				if (std::filesystem::create_directory(tempPath)) {
				}
				else {
					std::cerr << "unable to create temp directory " << tempPath.string() << std::endl;
					exit(1);
				}
			}
			tempdir = tempPath.string();

			if (tempdir2.empty()) {
				tempdir2 = tempdir;
			}

			std::filesystem::path tempPath2 = std::filesystem::path(tempdir2);
			if (std::filesystem::exists(tempPath2)) {
				if (std::filesystem::is_regular_file(tempPath2)) {
					tempPath2 = tempPath2.parent_path();
				}
			}
			else {
				if (std::filesystem::create_directory(tempPath2)) {
				}
				else {
					std::cerr << "unable to create temp2 directory " << tempPath2.string() << std::endl;
					exit(1);
				}
			}
			tempdir2 = tempPath2.string();
		}
		catch(const std::runtime_error& re)
		{
			std::cerr << "Runtime error: " << re.what() << std::endl;
			exit(1);
		}
		catch(const std::exception& ex)
		{
			std::cerr << "Error occurred: " << ex.what() << std::endl;
			exit(1);
		}
		catch(...)
		{
			std::cerr << "Unknown failure occurred. Possible memory corruption" << std::endl;
			exit(1);
		}

        cout << "Generating plot for k=" << static_cast<int>(k) << " filename=" << filename
             << " id=" << id << endl
             << endl;

        id = Strip0x(id);
        if (id.size() != 64) {
            cout << "Invalid ID, should be 32 bytes (hex)" << endl;
            exit(1);
        }
        memo = Strip0x(memo);
        if (memo.size() % 2 != 0) {
            cout << "Invalid memo, should be only whole bytes (hex)" << endl;
            exit(1);
        }
        std::vector<uint8_t> memo_bytes(memo.size() / 2);
        std::array<uint8_t, 32> id_bytes;

        HexToBytes(memo, memo_bytes.data());
        HexToBytes(id, id_bytes.data());

        DiskPlotter plotter = DiskPlotter();
        uint8_t phases_flags = 0;
        if (!nobitfield) {
            phases_flags = ENABLE_BITFIELD;
        }
        if (show_progress) {
            phases_flags = phases_flags | SHOW_PROGRESS;
        }
        plotter.CreatePlotDisk(
                tempdir,
                tempdir2,
                finaldir,
                filename,
                k,
                memo_bytes.data(),
                memo_bytes.size(),
                id_bytes.data(),
                id_bytes.size(),
                buffmegabytes,
                num_buckets,
                num_stripes,
                num_threads,
                phases_flags,
				show_progress);
    } else if (operation == "prove") {
        if (argc < 3) {
            HelpAndQuit(options);
        }
        cout << "Proving using filename=" << filename << " challenge=" << argv[2] << endl
             << endl;
        string challenge = Strip0x(argv[2]);
        if (challenge.size() != 64) {
            cout << "Invalid challenge, should be 32 bytes" << endl;
            exit(1);
        }
        uint8_t challenge_bytes[32];
        HexToBytes(challenge, challenge_bytes);

        DiskProver prover(filename);
        try {
            vector<LargeBits> qualities = prover.GetQualitiesForChallenge(challenge_bytes);
            for (uint32_t i = 0; i < qualities.size(); i++) {
                k = prover.GetSize();
                uint8_t *proof_data = new uint8_t[8 * k];
                LargeBits proof = prover.GetFullProof(challenge_bytes, i);
                proof.ToBytes(proof_data);
                cout << "Proof: 0x" << Util::HexStr(proof_data, k * 8) << endl;
                delete[] proof_data;
            }
            if (qualities.empty()) {
                cout << "No proofs found." << endl;
                exit(1);
            }
        } catch (const std::exception& ex) {
            std::cout << "Error proving. " << ex.what() << std::endl;
            exit(1);
        } catch (...) {
            std::cout << "Error proving. " << std::endl;
            exit(1);
        }
    } else if (operation == "verify") {
        if (argc < 4) {
            HelpAndQuit(options);
        }
        Verifier verifier = Verifier();

        id = Strip0x(id);
        string proof = Strip0x(argv[2]);
        string challenge = Strip0x(argv[3]);
        if (id.size() != 64) {
            cout << "Invalid ID, should be 32 bytes" << endl;
            exit(1);
        }
        if (challenge.size() != 64) {
            cout << "Invalid challenge, should be 32 bytes" << endl;
            exit(1);
        }
        if (proof.size() % 16) {
            cout << "Invalid proof, should be a multiple of 8 bytes" << endl;
            exit(1);
        }
        k = proof.size() / 16;
        cout << "Verifying proof=" << argv[2] << " for challenge=" << argv[3]
             << " and k=" << static_cast<int>(k) << endl
             << endl;
        uint8_t id_bytes[32];
        uint8_t challenge_bytes[32];
        uint8_t *proof_bytes = new uint8_t[proof.size() / 2];
        HexToBytes(id, id_bytes);
        HexToBytes(challenge, challenge_bytes);
        HexToBytes(proof, proof_bytes);

        LargeBits quality =
            verifier.ValidateProof(id_bytes, k, challenge_bytes, proof_bytes, k * 8);
        if (quality.GetSize() == 256) {
            cout << "Proof verification succeeded. Quality: " << quality << endl;
        } else {
            cout << "Proof verification failed." << endl;
            exit(1);
        }
        delete[] proof_bytes;
    } else if (operation == "check") {
        uint32_t iterations = 1000;
        if (argc == 3) {
            iterations = std::stoi(argv[2]);
        }

        DiskProver prover(filename);
        Verifier verifier = Verifier();

        uint32_t success = 0;
        uint8_t id_bytes[32];
        prover.GetId(id_bytes);
        k = prover.GetSize();

        for (uint32_t num = 0; num < iterations; num++) {
            vector<unsigned char> hash_input = intToBytes(num, 4);
            hash_input.insert(hash_input.end(), &id_bytes[0], &id_bytes[32]);

            vector<unsigned char> hash(picosha2::k_digest_size);
            picosha2::hash256(hash_input.begin(), hash_input.end(), hash.begin(), hash.end());

            try {
                vector<LargeBits> qualities = prover.GetQualitiesForChallenge(hash.data());

                for (uint32_t i = 0; i < qualities.size(); i++) {
                    LargeBits proof = prover.GetFullProof(hash.data(), i);
                    uint8_t *proof_data = new uint8_t[proof.GetSize() / 8];
                    proof.ToBytes(proof_data);
                    cout << "i: " << num << std::endl;
                    cout << "challenge: 0x" << Util::HexStr(hash.data(), 256 / 8) << endl;
                    cout << "proof: 0x" << Util::HexStr(proof_data, k * 8) << endl;
                    LargeBits quality =
                        verifier.ValidateProof(id_bytes, k, hash.data(), proof_data, k * 8);
                    if (quality.GetSize() == 256 && quality == qualities[i]) {
                        cout << "quality: " << quality << endl;
                        cout << "Proof verification suceeded. k = " << static_cast<int>(k) << endl;
                        success++;
                    } else {
                        cout << "Proof verification failed." << endl;
                    }
                    delete[] proof_data;
                }
            } catch (const std::exception& error) {
                cout << "Threw: " << error.what() << endl;
                continue;
            }
        }
        std::cout << "Total success: " << success << "/" << iterations << ", "
                  << (success * 100 / static_cast<double>(iterations)) << "%." << std::endl;
        if (show_progress) { progress(4, 1, 1); }
    } else {
        cout << "Invalid operation. Use create/prove/verify/check" << endl;
    }
    return 0;
} catch (const cxxopts::OptionException &e) {
    cout << "error parsing options: " << e.what() << endl;
    return 1;
} catch (const std::exception &e) {
    std::cerr << "Caught exception: " << e.what() << endl;
    throw e;
}
