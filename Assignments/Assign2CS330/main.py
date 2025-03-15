# CS 330 Simon Liang , Ben Richeson
from threading import Thread, Lock
from time import sleep
import random


class BankAccount:
    bankID:int
    amount:int
    
    
    def __init__ (self,bankID):
        self.bankID = bankID
        self.balance = 0
        self.transactions = 0
        self.lock = Lock()
        

    def deposit(self,amount:int):
        if self.lock.locked:
            print (f"Still processing the previous transaction")


        with self.lock:
            self.balance += amount
            print(f"Depositing {amount} to Account {self.bankID}. Balance: {self.balance}")
            self.transactions +=1
        
    
    def withdraw(self,amount:int):
        if self.lock.locked:
            print("Still processing the previous transaction")
        with self.lock:
            if self.balance >= amount:
                self.balance = self.balance - amount
                print(f"Withdrawn {amount} from Account {self.bankID}. Balance: {self.balance}")
                self.transactions +=1
            else:
                print(f"Insufficient Balance to withdraw {amount}. Balance in Account {self.bankID} is {self.balance}")
                
        

class DepositThread(Thread):

    account : BankAccount
    amount : int
    def __init__(self, account:BankAccount, amount:int):
        Thread.__init__(self)
        self.account = account
        self.amount = amount

    def run(self):
        self.account.deposit(self.amount)


class WithdrawThread(Thread):
    def __init__(self,account:BankAccount, amount:int):
        Thread.__init__(self)
        self.account = account
        self.amount = amount

    def run(self):
        self.account.withdraw(self.amount)


if __name__ == '__main__':
    ACCOUNTS = 3
    TRANSACTIONS_D_W = ACCOUNTS * 3
    MIN_TRANSACTION = 20
    MAX_TRANSACTION = 100
    

    bank_accounts: list[BankAccount] = [
        BankAccount(i) for i in range(0, ACCOUNTS)
    ]

    threads: list[Thread] = []

    for i in range(0, TRANSACTIONS_D_W):

        ba = bank_accounts[i % ACCOUNTS]
        
        thrD = (DepositThread)(ba, random.randint(MIN_TRANSACTION,MAX_TRANSACTION))

        thrW = (WithdrawThread)(ba, random.randint(MIN_TRANSACTION,MAX_TRANSACTION))
        print(f"{thrD}")
        threads.append(thrD)
        thrD.start()
        print(f"{thrW}")
        threads.append(thrW)
        thrW.start()

    # block until all the threads finish
    for thr in threads:
        thr.join()


    for ba in bank_accounts:
        print(f"[Bank ID {ba.bankID}] Balance:{ba.balance} Transactions made:{ba.transactions}")
    
    


    
